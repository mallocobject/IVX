#include "edit_layer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "ivx/core/application.h"
#include "ivx/core/input.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/render/render_command.h"
#include "ivx/scene/scene.h"
#include "ivx/scene/scriptable_entity.h"
#include "ivx/serialize/serializer.h"
#include "panel/component_registry.h"
#include "panel/layout.h"
#include "scripts/script_registry.h"
#include "ui/theme.h"

#include <imgui.h>
// -- -- -- -- -- -- -- -- -- -- -
#include <ImGuizmo.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>

#include <imgui_internal.h> // FindWindowByName (viewport focus after a layout reset)

namespace ivx {

static constexpr uint32_t WIDTH = 1280;
static constexpr uint32_t HEIGHT = 720;

EditLayer::EditLayer() : Layer("EditLayer") {
}

void EditLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    // The editor layout is stored next to the assets instead of the process
    // working directory, so it survives being launched from anywhere.
    Application::get().imgui_layer()->set_ini_filename(LAYOUT);
    // A saved ini means the user already has a layout: do not rebuild the
    // default one over it (Reset Layout in the Layout menu still does).
    layout_initialized_ = std::filesystem::exists(LAYOUT);

    // Photoshop-style dark theme plus the two faces: proportional for the
    // chrome, monospace for code and logs. Both live in ui/theme.cpp.
    ui::apply_theme();
    ui::load_fonts();

    // auto texture = Texture2D::create(TEXTURES "tilemap.png");
    // auto sub_texture =
    //     SubTexture2D::create(texture, {1.f, 1.f}, {16.f, 16.f}, {1.f, 1.f});

    FrameBufferSpecification spec;
    spec.width = WIDTH;
    spec.height = HEIGHT;
    spec.num_samples = 4;
    spec.attachments = {{FrameBufferTextureFormat::RGBA8}};

    frame_buffer_ = FrameBuffer::create(spec);

    // The editor starts without a scene: the user picks File > New or File >
    // Open. An auto-created empty scene looks the same as a broken one, and
    // entities added to it had no file to belong to; the hierarchy panel
    // refuses to create entities until a scene exists.
    scene_.reset();
    context_.scene.reset();

    setup_panels();
}

void EditLayer::on_detach() {
    IVX_PROFILE_FUNCTION();

    // Leave the global gate open for whatever runs after the editor layer.
    Input::set_enabled(true);
}

void EditLayer::on_update(float dt) {
    IVX_PROFILE_FUNCTION();

    // Keeps collecting log output even when the Console panel is closed.
    if (console_panel_) {
        console_panel_->pump();
    }

    if (visible_) {
        const auto &spec = frame_buffer_->get_specification();
        if (spec.width != viewport_size_.x || spec.height != viewport_size_.y) {
            uint32_t w = static_cast<uint32_t>(viewport_size_.x);
            uint32_t h = static_cast<uint32_t>(viewport_size_.y);

            if (scene_) {
                scene_->on_viewport_resize(w, h);
            }
            frame_buffer_->resize(w, h);
            editor_camera_.set_viewport_size(w, h);
        }
    }

    if (scene_) {
        scene_->on_update(dt);
    }

    editor_camera_.on_update(dt);

    // render
    Renderer2D::reset_stats();

    frame_buffer_->bind();

    {
        IVX_PROFILE_SCOPE("Renderer prep");
        frame_buffer_->clear(glm::vec4{0.1f, 0.1f, 0.1f, 1.f});
    }

    if (scene_) {
        // draw
        IVX_PROFILE_SCOPE("Renderer draw");

        scene_->on_render(editor_camera_);
    }

    frame_buffer_->unbind();
}

void EditLayer::on_event(Event &event) {
    // Scripts see the same events the rest of the app sees, but only while the
    // viewport owns the input. The rule has to match the polling one exactly:
    // with focus-only gating here and hover-based gating there, the wheel
    // (event) died while WASD (polled) kept working.
    if (input_owned_) {
        if (scene_) {
            scene_->on_event(event);
        }
        editor_camera_.on_event(event);
    }
}

void EditLayer::on_imgui() {
    IVX_PROFILE_FUNCTION();

    // ImGuizmo wants one BeginFrame() per UI frame, before any panel draws:
    // without it IsOver()/IsUsing() never turn true and the handles are inert.
    ImGuizmo::BeginFrame();

    // Top: the menu bar reserves its own height in the main viewport's work
    // area (and it owns the "Reset Layout" flag, so it runs before the
    // dockspace).
    draw_menu_bar();

    // Bottom: same trick, so the dockspace below stops above the status bar.
    draw_status_bar();

    // replace fullscreen host window + DockSpace
    const ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(
        0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    // First frame (or after "Reset Layout") build the default arrangement.
    if (!layout_initialized_ || reset_layout_requested_) {
        setup_default_layout(dockspace_id);
        layout_initialized_ = true;
        reset_layout_requested_ = false;
        // The dock builder leaves the last docked window selected; the viewport
        // is what you want to see on startup. Applied once the panels exist,
        // i.e. on the next frame (see below).
        pending_viewport_focus_ = true;
    }

    // --- UI / ImGui submission ---

    panels_.on_render();

    // Bring the viewport tab to the front after a layout build. Doing it here
    // (not next to setup_default_layout) means the panels were submitted at
    // least once, so the window exists, and focusing it also makes the camera
    // keys work right after startup.
    if (pending_viewport_focus_) {
        if (ImGui::FindWindowByName("Viewport") != nullptr) {
            pending_viewport_focus_ = false;
            ImGui::SetWindowFocus("Viewport");
        }
    }

    // The input gate is driven by the viewport focus sampled while drawing the
    // panel. If the panel did not draw this frame (closed, collapsed, or the
    // window hidden) nobody owns the keyboard, and a stale `true` here would
    // leave input flowing into game scripts.
    if (!viewport_sampled_) {
        viewport_focused_ = false;
        viewport_hovered_ = false;
        Input::set_enabled(false);
    }
    viewport_sampled_ = false;
}

void EditLayer::create_scene() {
    scene_ = make_ref<Scene>();
    context_.scene = scene_;
    context_.clear_selection();

    {
        Entity cube_entity = scene_->create_entity("Cube");
        auto &tf = cube_entity.get_component<TransformComponent>();
        tf.translation = glm::vec3{0.f};
        tf.rotation = glm::vec3{0.f};
        tf.scale = glm::vec3{1.f};

        // front
        Entity front = scene_->create_entity("front piece");
        {
            auto &tf = front.get_component<TransformComponent>();
            tf.translation = glm::vec3{0.f, 0.f, 200.f};
            tf.rotation = glm::vec3{0.f};
            tf.scale = glm::vec3{1.f};
            auto &sr = front.add_component<SpriteRendererComponent>();
            sr.color = glm::vec4{0.95f};
            sr.size = glm::vec2{400.f};
        }

        // back
        Entity back = scene_->create_entity("back piece");
        {
            auto &tf = back.get_component<TransformComponent>();
            tf.translation = glm::vec3{0.f, 0.f, -200.f};
            tf.rotation = glm::vec3{0.f, glm::radians(180.f), 0.f};
            tf.scale = glm::vec3{1.f};
            auto &sr = back.add_component<SpriteRendererComponent>();
            sr.color = glm::vec4{0.55f};
            sr.size = glm::vec2{400.f};
        }

        // left
        Entity left = scene_->create_entity("left piece");
        {
            auto &tf = left.get_component<TransformComponent>();
            tf.translation = glm::vec3{-200.f, 0.f, 0.f};
            tf.rotation = glm::vec3{0.f, glm::radians(-90.f), 0.f};
            tf.scale = glm::vec3{1.f};
            auto &sr = left.add_component<SpriteRendererComponent>();
            sr.color = glm::vec4{0.70f};
            sr.size = glm::vec2{400.f};
        }

        // right
        Entity right = scene_->create_entity("right piece");
        {
            auto &tf = right.get_component<TransformComponent>();
            tf.translation = glm::vec3{200.f, 0.f, 0.f};
            tf.rotation = glm::vec3{0.f, glm::radians(90.f), 0.f};
            tf.scale = glm::vec3{1.f};
            auto &sr = right.add_component<SpriteRendererComponent>();
            sr.color = glm::vec4{0.85f};
            sr.size = glm::vec2{400.f};
        }

        // top
        Entity top = scene_->create_entity("top piece");
        {
            auto &tf = top.get_component<TransformComponent>();
            tf.translation = glm::vec3{0.f, 200.f, 0.f};
            tf.rotation = glm::vec3{glm::radians(-90.f), 0.f, 0.f};
            tf.scale = glm::vec3{1.f};
            auto &sr = top.add_component<SpriteRendererComponent>();
            sr.color = glm::vec4{1.00f};
            sr.size = glm::vec2{400.f};
        }

        // bottom
        Entity bottom = scene_->create_entity("bottom piece");
        {
            auto &tf = bottom.get_component<TransformComponent>();
            tf.translation = glm::vec3{0.f, -200.f, 0.f};
            tf.rotation = glm::vec3{glm::radians(90.f), 0.f, 0.f};
            tf.scale = glm::vec3{1.f};
            auto &sr = bottom.add_component<SpriteRendererComponent>();
            sr.color = glm::vec4{0.40f};
            sr.size = glm::vec2{400.f};
        }

        scene_->set_parent(front, cube_entity);
        scene_->set_parent(back, cube_entity);
        scene_->set_parent(left, cube_entity);
        scene_->set_parent(right, cube_entity);
        scene_->set_parent(top, cube_entity);
        scene_->set_parent(bottom, cube_entity);
    }

    if (viewport_size_.x > 0.f && viewport_size_.y > 0.f) {
        scene_->on_viewport_resize(static_cast<uint32_t>(viewport_size_.x),
                                   static_cast<uint32_t>(viewport_size_.y));
    }
}

void EditLayer::load_scene(const std::filesystem::path &filepath) {
    Ref<Scene> loaded = Serializer::deserialize(filepath);
    if (!loaded) {
        return; // Serializer already logged why
    }

    // The whole scene object is replaced: cameras, scripts and the selection
    // all point into the old one. Panels read context_.scene, so they follow
    // along.
    scene_ = std::move(loaded);
    context_.scene = scene_;
    context_.clear_selection();

    if (viewport_size_.x > 0.f && viewport_size_.y > 0.f) {
        scene_->on_viewport_resize(static_cast<uint32_t>(viewport_size_.x),
                                   static_cast<uint32_t>(viewport_size_.y));
    }
}

void EditLayer::save_scene(const std::filesystem::path &filepath) {
    if (!scene_) {
        return;
    }
    Serializer::serialize(scene_, filepath);
}

void EditLayer::setup_panels() {
    IVX_PROFILE_FUNCTION();

    register_all_components();

    // Deterministic draw order: Hierarchy -> Viewport -> Inspector -> Camera.
    scene_hierarchy_panel_ = panels_.add<SceneHierarchyPanel>(context_);

    auto viewport = panels_.add<LambdaPanel>(
        "Viewport", [this]() { draw_viewport_content(); });
    viewport
        ->set_pre_begin([]() {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
        })
        .set_post_end([]() { ImGui::PopStyleVar(); })
        .set_on_hidden([this]() {
            visible_ = false;
            viewport_focused_ = false;
            viewport_hovered_ = false;
            Application::get().imgui_layer()->block_event(true);
        });
    viewport_panel_ = viewport;

    inspector_panel_ = panels_.add<InspectorPanel>(context_);
    console_panel_ = panels_.add<ConsolePanel>();
}

void EditLayer::draw_menu_bar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("File")) {
        // New / Open are the only way to get a scene, so they must stay enabled
        // while there is none; only the actions that need one check for it.
        if (ImGui::MenuItem("New")) {
            create_scene();
        }
        if (ImGui::MenuItem("Open")) {
            if (auto filepath =
                    FileDialog::open_file("IVX Scene (*.ivx)\0*.ivx\0");
                !filepath.empty()) {
                load_scene(filepath);
            }
        }
        if (ImGui::MenuItem("Save as ...", nullptr, false, scene_ != nullptr)) {
            if (auto filepath =
                    FileDialog::save_file("IVX Scene (*.ivx)\0*.ivx\0");
                !filepath.empty()) {
                save_scene(filepath);
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
            Application::get().close();
        }
        ImGui::EndMenu();
    }
    panels_.draw_view_menu();
    if (ImGui::BeginMenu("Gizmo")) {
        // Same three modes as the numeric shortcuts; the checkmark doubles as
        // the current state readout (there is no room for one in the status
        // bar).
        if (ImGui::MenuItem(
                "Translate", "1", gizmo_mode_ == GizmoMode::Translate)) {
            gizmo_mode_ = GizmoMode::Translate;
        }
        if (ImGui::MenuItem("Rotate", "2", gizmo_mode_ == GizmoMode::Rotate)) {
            gizmo_mode_ = GizmoMode::Rotate;
        }
        if (ImGui::MenuItem("Scale", "3", gizmo_mode_ == GizmoMode::Scale)) {
            gizmo_mode_ = GizmoMode::Scale;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Layout")) {
        if (ImGui::MenuItem("Reset Layout")) {
            reset_layout_requested_ = true;
        }
        if (ImGui::BeginMenu("UI Scale")) {
            // Fonts and spacings scale together (see ui::set_scale), and the
            // choice is stored next to the editor assets so it survives a
            // restart.
            for (const float option : {0.9f, 1.f, 1.1f, 1.2f, 1.3f, 1.5f}) {
                char label[16];
                std::snprintf(label, sizeof(label), "%.0f%%", option * 100.f);
                const bool selected = std::abs(ui::scale() - option) < 0.001f;
                if (ImGui::MenuItem(label, nullptr, selected)) {
                    ui::set_scale(option);
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void EditLayer::draw_status_bar() {
    // A thin strip on the bottom edge of the main viewport.
    // BeginViewportSideBar reports its size back into the viewport's work area,
    // which is why the dockspace above it never overlaps.
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    const float height = ImGui::GetFrameHeight() + 2.f;

    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          ImVec4{0.149f, 0.149f, 0.149f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.62f, 0.62f, 0.62f, 1.f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{10.f, 3.f});

    if (ImGui::BeginViewportSideBar("##status_bar",
                                    viewport,
                                    ImGuiDir_Down,
                                    height,
                                    ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoSavedSettings)) {
        // --- left: what is selected, then how big the scene is
        if (context_.has_selection()) {
            const Entity selected = context_.selected_entity();
            const auto &tag = selected.get_component<TagComponent>();
            ImGui::TextUnformatted(tag.value.c_str());

        } else {
            ImGui::TextDisabled("No selection");
        }

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::Text("%zu object(s)",
                    scene_ ? scene_->registry().view<TagComponent>().size()
                           : std::size_t{0});

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::Text("%.0fx%.0f viewport", viewport_size_.x, viewport_size_.y);

        // --- right: what the viewport is doing and how fast it is doing it
        //
        // Both blocks are laid out from the right edge so the left side can
        // grow (a long entity name) without pushing them off screen. The gizmo
        // mode lives here because it is the one piece of state with no other
        // readout.
        const char *const gizmo_name =
            !context_.has_selection()             ? "None"
            : gizmo_mode_ == GizmoMode::Translate ? "Move"
            : gizmo_mode_ == GizmoMode::Rotate    ? "Rotate"
                                                  : "Scale";

        const ImGuiIO &io = ImGui::GetIO();
        char fps_text[64];
        std::snprintf(fps_text,
                      sizeof(fps_text),
                      "%.0f FPS  %.2f ms",
                      io.Framerate,
                      io.Framerate > 0.f ? 1000.f / io.Framerate : 0.f);

        const ImGuiStyle &style = ImGui::GetStyle();
        const float separator_width = ImGui::CalcTextSize("|").x;
        const float block_width = ImGui::CalcTextSize(gizmo_name).x +
                                  ImGui::CalcTextSize(fps_text).x +
                                  2.f * separator_width +
                                  4.f * style.ItemSpacing.x;
        ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - block_width);
        ImGui::TextUnformatted(gizmo_name);
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextUnformatted(fps_text);
    }
    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void EditLayer::draw_viewport_content() {
    viewport_focused_ = ImGui::IsWindowFocused();
    viewport_hovered_ = ImGui::IsWindowHovered();
    viewport_sampled_ = true;

    // The game view owns the input while it is focused OR hovered (and no text
    // field is active), the way Unity behaves. Focus-only was too strict:
    // clicking the console, the inspector or a script silently killed WASD
    // *and* the wheel until the viewport was clicked again.
    const bool hover_owns_input =
        viewport_hovered_ && !ImGui::GetIO().WantTextInput;
    const bool owns_input = viewport_focused_ || hover_owns_input;

    Input::set_enabled(owns_input);
    // Latched for on_event(): the event path runs before this frame's UI, so it
    // has to read the decision from the previous frame.
    input_owned_ = owns_input;

    // Both routes have to agree: the polling path above and the event path
    // below. When they disagreed, the wheel (an event, not a polled key) was
    // swallowed by the editor UI while WASD kept working.
    Application::get().imgui_layer()->block_event(!owns_input);

    visible_ = true;

    ImVec2 viewport_size = ImGui::GetContentRegionAvail();
    float w = std::max(viewport_size.x, 1.f);
    float h = std::max(viewport_size.y, 1.f);

    viewport_size_ = {w, h};

    ImGui::Image(
        static_cast<ImTextureID>(frame_buffer_->get_color_attachment()),
        viewport_size_,
        ImVec2{0.0f, 1.0f},
        ImVec2{1.0f, 0.0f});

    // Gizmo handles go on top of the image, in its exact rectangle.
    draw_gizmo(ImGui::GetItemRectMin(), ImGui::GetItemRectSize());
}

int EditLayer::gizmo_operation() const {
    switch (gizmo_mode_) {
    case GizmoMode::Translate:
        return ImGuizmo::TRANSLATE;
    case GizmoMode::Rotate:
        return ImGuizmo::ROTATE;
    case GizmoMode::Scale:
        return ImGuizmo::SCALE;
    default:
        IVX_CORE_ASSERT(false, "Unsupported operation!");
        return 0;
    }
}

void EditLayer::draw_gizmo(const ImVec2 &image_min, const ImVec2 &image_size) {
    if (!scene_ || !context_.has_selection()) {
        return;
    }

    Entity selected = context_.selected_entity();
    auto &transform = selected.get_component<TransformComponent>();

    // The viewport renders with the primary camera, so the gizmo has to use
    // the same pair of matrices or the handles drift away from the
    // geometry. glm::mat4 view{1.f}; glm::mat4 projection{1.f}; bool
    // orthographic = true; bool found = false; for (auto [handle, camera,
    // camera_transform] :
    //      scene_->registry()
    //          .view<CameraComponent, TransformComponent>()
    //          .each()) {
    //     if (!camera.primary) {
    //         continue;
    //     }
    //     projection = camera.camera.get_proj_matrix();
    //     view = glm::inverse(camera_transform.world);
    //     orthographic = camera.camera.get_projection_type() ==
    //                    SceneCamera::ProjectionType::Orthographic;
    //     found = true;
    //     break;
    // }
    // if (!found) {
    //     return;
    // }

    if (input_owned_) {
        if (Input::is_key_pressed(KeyCode::D1)) {
            gizmo_mode_ = GizmoMode::Translate;
        }
        if (Input::is_key_pressed(KeyCode::D2)) {
            gizmo_mode_ = GizmoMode::Rotate;
        }
        if (Input::is_key_pressed(KeyCode::D3)) {
            gizmo_mode_ = GizmoMode::Scale;
        }
    }

    float snap_value[3]{0.f};
    float *snap_ptr = nullptr;
    if (Input::is_key_pressed(KeyCode::LeftControl)) {
        switch (gizmo_operation()) {
        case ImGuizmo::TRANSLATE:
            snap_value[0] = snap_value[1] = snap_value[2] = 50.f;
            snap_ptr = snap_value;
            break;
        case ImGuizmo::ROTATE:
            snap_value[0] = 45.0f;
            snap_ptr = snap_value;
            break;
        case ImGuizmo::SCALE:
            snap_value[0] = 0.1f;
            snap_ptr = snap_value;
            break;
        }
    }

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(image_min.x, image_min.y, image_size.x, image_size.y);

    // ImGuizmo wants the view and the projection separately; handing it a
    // view-projection matrix applies the view twice and the handles drift
    // off the entity.
    glm::mat4 view = editor_camera_.get_view_matrix();
    glm::mat4 projection = editor_camera_.get_proj_matrix();

    glm::mat4 model = transform.world;

    if (int op = gizmo_operation()) {
        ImGuizmo::Manipulate(glm::value_ptr(view),
                             glm::value_ptr(projection),
                             static_cast<ImGuizmo::OPERATION>(op),
                             ImGuizmo::LOCAL,
                             glm::value_ptr(model),
                             nullptr,
                             snap_ptr);
    }

    if (ImGuizmo::IsUsing()) {
        apply_gizmo_matrix(selected, model);
    }
}

void EditLayer::apply_gizmo_matrix(Entity entity,
                                   const glm::mat4 &world_matrix) {
    auto &transform = entity.get_component<TransformComponent>();

    // The gizmo works in world space; the component stores a local,
    // parent-relative pose.
    glm::mat4 parent_world{1.f};
    if (Entity parent = entity.parent()) {
        parent_world = parent.get_component<TransformComponent>().world;
    }
    const glm::mat4 local = glm::inverse(parent_world) * world_matrix;

    glm::vec3 translation{0.f};
    glm::vec3 unused_rotation{0.f};
    glm::vec3 scale{1.f};
    decompose_transform(local, translation, unused_rotation, scale);

    // ???
    const glm::vec3 previous_rotation = transform.rotation;
    const glm::mat3 delta_world = rotation_of(world_matrix) *
                                  glm::transpose(rotation_of(transform.world));
    const glm::mat3 parent_rotation = rotation_of(parent_world);
    const glm::quat increment = glm::normalize(glm::quat_cast(
        glm::transpose(parent_rotation) * delta_world * parent_rotation));
    const glm::quat current = glm::normalize(glm::quat_cast(
        rotation_of(detail::build_rotation(transform.rotation))));
    const glm::quat updated = glm::normalize(increment * current);
    const glm::vec3 rotation =
        euler_from_rotation_matrix(glm::mat3_cast(updated));

    transform.translation = translation;
    transform.scale = scale;
    transform.rotation =
        unwrap_euler(nearest_equivalent_euler(rotation, previous_rotation),
                     previous_rotation);
}
} // namespace ivx
