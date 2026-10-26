#include "edit_layer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/application.h"
#include "ivx/core/input.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/renderer/render_command.h"
#include "ivx/scene/scene.h"
#include "ivx/scene/scriptable_entity.h"
#include "panel/component_registry.h"
#include "panel/layout.h"
#include "scripts/script_registry.h"
#include "ui/theme.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h> // FindWindowByName (viewport focus after a layout reset)

namespace ivx {

static constexpr uint32_t WIDTH = 1280;
static constexpr uint32_t HEIGHT = 720;

EditLayer::EditLayer() : Layer("EditLayer") {
}

void EditLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    // Script module first: its registry has to exist before anything can bind a
    // script. A missing DLL is not fatal, the editor just has no script types.
    // vcvars64.bat costs a couple of seconds, so start capturing the MSVC
    // environment now: by the time the first build runs it is usually ready.
    warm_up_script_build_environment();

    {
        std::string error;
        if (!scripts_.load_initial(error)) {
            IVX_WARN("Script module not loaded ({}), building it", error);
            // The module lives in its own build directory, so a fresh tree has
            // no DLL yet. Kick off the first build; poll_scripts() loads it
            // when it is done.
            scripts_.begin_build();
        }
    }

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

    // static uint32_t WIDTH = Application::get().get_window().get_width();
    // static uint32_t HEIGHT = Application::get().get_window().get_height();

    texture_ = Texture2D::create(TEXTURES "tilemap.png");
    sub_texture_ =
        SubTexture2D::create(texture_, {2.f, 1.f}, {16.f, 16.f}, {2.f, 1.f});

    FrameBufferSpecification spec;
    spec.width = WIDTH;
    spec.height = HEIGHT;

    frame_buffer_ = FrameBuffer::create(spec);

    scene_ = make_ref<Scene>();
    context_.scene = scene_;
    square_entity_ = scene_->create_entity("Colorful Square");

    // auto transform = [](const glm::vec3 &translation = glm::vec3{0.f},
    //                     float rotation = 0.f,
    //                     const glm::vec2 &scale = glm::vec2{1.f}) -> glm::mat4
    //                     {
    //     return glm::translate(glm::mat4{1.f}, translation) *
    //            glm::rotate(glm::mat4{1.f},
    //                        glm::radians(rotation),
    //                        glm::vec3{0.f, 0.f, 1.f}) *
    //            glm::scale(glm::mat4{1.f}, glm::vec3{scale, 1.f});
    // };

    // Scale stays 1: the quad size lives in SpriteRendererComponent so that
    // children are not scaled by their parent.
    square_entity_.get_component<TransformComponent>().translation =
        glm::vec3{0.f};
    square_entity_.get_component<TransformComponent>().rotation = 0.f;
    square_entity_.get_component<TransformComponent>().scale =
        glm::vec2{1.f, 1.f};

    square_entity_.add_component<SpriteRendererComponent>(
        sub_texture_,
        1.f,
        glm::vec4{1.f, 1.f, 1.f, 1.f},
        glm::vec2{400.f, 200.f});

    // A demo camera entity: nothing special about it, it simply owns a camera
    // component and is marked primary. Any other entity could do the same.
    {
        Entity camera_entity = scene_->create_entity("Camera Entity");
        auto &camera = camera_entity.add_component<CameraComponent>();
        camera.camera.set_orthographic_size(static_cast<float>(HEIGHT));
        camera.camera.set_projection_type(
            SceneCamera::ProjectionType::Orthographic);
        camera.primary = true;
        // Initial pose: one unit in front of the sprite plane (z = 0), looking
        // down -Z. near/far are distances, so 0.01 .. 1000 covers it.
        camera_entity.get_component<TransformComponent>().translation =
            glm::vec3{0.f, 0.f, 1.f};
    }

    // The script host needs the scene to re-bind scripts after a hot reload.
    scripts_.set_scene(scene_.get());
    bind_default_camera_script();

    setup_panels();
}

void EditLayer::bind_default_camera_script() {
    // The scene is rebuilt from code on every start, so a hand-made binding in
    // the inspector does not survive a restart. Give the *primary* camera its
    // controller automatically, otherwise WASD/wheel silently do nothing.
    //
    // Keyed off "whichever entity owns the primary camera", not off a fixed
    // entity: a camera is just a component, so it may live on a moving player
    // instead.
    //
    // Only once per session and only when nothing is bound: unbinding it in the
    // inspector has to stick.
    if (default_script_bound_ || !scene_) {
        return;
    }

    Entity camera_entity{};
    for (auto [handle, camera] :
         scene_->registry().view<CameraComponent>().each()) {
        if (camera.primary) {
            camera_entity = Entity{handle, scene_.get()};
            break;
        }
    }
    if (!camera_entity) {
        return;
    }

    auto *existing = camera_entity.try_get_component<NativeScriptComponent>();
    if (existing != nullptr && existing->is_bound()) {
        default_script_bound_ = true;
        return;
    }

    const bool available =
        std::any_of(ScriptRegistry::get().entries().begin(),
                    ScriptRegistry::get().entries().end(),
                    [](const ScriptEntry &entry) {
                        return entry.name == "CameraController";
                    });
    if (!available) {
        // The module is still being built (fresh tree) or the script was
        // removed; poll_scripts() tries again once the build has finished.
        return;
    }

    if (existing == nullptr) {
        camera_entity.add_component<NativeScriptComponent>();
    }
    if (ScriptRegistry::get().bind_by_name("CameraController", camera_entity)) {
        const auto *tag = camera_entity.try_get_component<TagComponent>();
        IVX_INFO("'{}': bound CameraController (WASD move, Q/E rotate, wheel "
                 "zoom)",
                 tag != nullptr ? tag->tag : "camera");
        default_script_bound_ = true;
    }
}

void EditLayer::on_detach() {
    IVX_PROFILE_FUNCTION();

    // Script instances live inside VXScripts.dll: their vtable and their code
    // are gone once the module is unloaded. Scene::~Scene() calls on_destroy()
    // on whatever is still bound, and ~Scene runs *after* this layer's
    // ScriptHost member, so the instances have to be released here, while the
    // module is still mapped.
    scripts_.release_all_scripts();

    // Leave the global gate open for whatever runs after the editor layer.
    Input::set_enabled(true);
}

void EditLayer::on_update(float dt) {
    IVX_PROFILE_FUNCTION();

    const auto update_start = std::chrono::steady_clock::now();

    // Finalize the previous frame's measurements and start a new sample.
    if (stats_panel_) {
        stats_panel_->begin_frame(dt);
    }

    // Keeps collecting log output even when the Console panel is closed.
    if (console_panel_) {
        console_panel_->pump();
    }

    // Finish a script build that ran on the worker thread this frame: the
    // reload itself must happen here, on the main thread.
    poll_scripts();

    if (visible_) {
        const auto &spec = frame_buffer_->get_specification();
        if (spec.width != viewport_size_.x || spec.height != viewport_size_.y) {
            uint32_t w = static_cast<uint32_t>(viewport_size_.x);
            uint32_t h = static_cast<uint32_t>(viewport_size_.y);

            scene_->on_viewport_resize(w, h);
            frame_buffer_->resize(w, h);
        }
    }

    // Gate input, not script state: overwriting NativeScriptComponent::enabled
    // here used to silently undo the inspector's "Enabled" checkbox and made
    // "Unbind" look broken. Whether a script runs is the user's choice; whether
    // keyboard polling happens depends on viewport focus (set while drawing the
    // viewport panel, see draw_viewport_content()).

    scene_->on_update(dt);

    // render
    Renderer2D::reset_stats();

    frame_buffer_->bind();

    {
        // clear
        IVX_PROFILE_SCOPE("Renderer prep");
        RenderCommand::set_clear_color(glm::vec4{0.1f, 0.1f, 0.1f, 1.f});
        RenderCommand::clear();
    }

    {
        // draw
        IVX_PROFILE_SCOPE("Renderer draw");

        scene_->on_render();
    }

    frame_buffer_->unbind();

    if (stats_panel_) {
        const auto update_end = std::chrono::steady_clock::now();
        const float ms =
            std::chrono::duration<float, std::milli>(update_end - update_start)
                .count();
        stats_panel_->submit("update", ms);
    }
}

void EditLayer::reload_scripts() {
    IVX_PROFILE_FUNCTION();

    // Non-blocking: the build runs on a worker thread and is picked up by
    // poll_scripts() in on_update().
    if (!scripts_.begin_build()) {
        IVX_WARN("Script build already running");
    }
}

void EditLayer::poll_scripts() {
    if (!scripts_.is_building()) {
        return;
    }

    scripts_.poll_build();
    if (scripts_.is_building()) {
        return; // still compiling
    }

    // A fresh tree has no DLL at startup, so the default camera binding could
    // not happen yet: retry now that the module is loaded.
    bind_default_camera_script();

    if (!scripts_.build_log().empty()) {
        IVX_INFO("Script build output:\n{}", scripts_.build_log());
    }
    if (!scripts_.last_build_ok()) {
        IVX_ERROR("Script rebuild failed");
    }
}

void EditLayer::on_event(Event &event) {
    // Scripts see the same events the rest of the app sees, but only while the
    // viewport owns the input. The rule has to match the polling one exactly:
    // with focus-only gating here and hover-based gating there, the wheel
    // (event) died while WASD (polled) kept working.
    if (input_owned_ && scene_) {
        scene_->on_event(event);
    }
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
    stats_panel_ = panels_.add<StatsPanel>();
    script_panel_ = panels_.add<ScriptPanel>(scripts_);
}

void EditLayer::draw_menu_bar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit")) {
            Application::get().close();
        }
        ImGui::EndMenu();
    }
    panels_.draw_view_menu();
    if (ImGui::BeginMenu("Scripts")) {
        if (ImGui::MenuItem("Rebuild & Reload")) {
            reload_scripts();
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
            if (const auto *tag = selected.try_get_component<TagComponent>()) {
                ImGui::TextUnformatted(tag->tag.c_str());
            } else {
                ImGui::TextUnformatted("(no tag)");
            }
        } else {
            ImGui::TextDisabled("No selection");
        }

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::Text("%zu object(s)",
                    scene_->registry().view<TagComponent>().size());

        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::Text("%.0fx%.0f viewport", viewport_size_.x, viewport_size_.y);

        // --- right: script module state, then frame timing
        char scripts_text[128];
        if (scripts_.is_building()) {
            std::snprintf(scripts_text,
                          sizeof(scripts_text),
                          "Scripts: building... %.1f s",
                          scripts_.build_seconds());
        } else if (!scripts_.is_loaded()) {
            std::snprintf(scripts_text, sizeof(scripts_text), "Scripts: none");
        } else if (!scripts_.build_log().empty() && !scripts_.last_build_ok()) {
            std::snprintf(
                scripts_text, sizeof(scripts_text), "Scripts: build failed");
        } else {
            std::snprintf(scripts_text,
                          sizeof(scripts_text),
                          "Scripts: %zu",
                          ScriptRegistry::get().entries().size());
        }

        const ImGuiIO &io = ImGui::GetIO();
        char fps_text[64];
        std::snprintf(fps_text,
                      sizeof(fps_text),
                      "%.0f FPS  %.2f ms",
                      io.Framerate,
                      io.Framerate > 0.f ? 1000.f / io.Framerate : 0.f);

        const float right_width = ImGui::CalcTextSize(scripts_text).x +
                                  ImGui::CalcTextSize(fps_text).x +
                                  ImGui::CalcTextSize("  |  ").x + 2.f;
        const float right_x = ImGui::GetWindowWidth() - right_width - 10.f;
        if (right_x > ImGui::GetCursorPosX() + 8.f) {
            ImGui::SameLine(right_x);
        } else {
            ImGui::SameLine();
        }

        if (scripts_.is_building()) {
            ImGui::TextColored(
                ImVec4{0.95f, 0.78f, 0.35f, 1.f}, "%s", scripts_text);
        } else if (!scripts_.is_loaded()) {
            ImGui::TextColored(
                ImVec4{0.90f, 0.45f, 0.45f, 1.f}, "%s", scripts_text);
        } else {
            ImGui::TextUnformatted(scripts_text);
        }
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextUnformatted(fps_text);
    }
    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}

void EditLayer::on_imgui() {
    IVX_PROFILE_FUNCTION();

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
    const auto ui_start = std::chrono::steady_clock::now();

    // The stats overlay is anchored to the viewport: hide it while the viewport
    // tab is not the visible one (writing a script, for instance), otherwise it
    // covers the code. One frame of lag, which is invisible in practice.
    if (stats_panel_ && viewport_panel_) {
        stats_panel_->set_host_visible(viewport_panel_->visible_this_frame());
    }

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

    if (stats_panel_) {
        const float ms = std::chrono::duration<float, std::milli>(
                             std::chrono::steady_clock::now() - ui_start)
                             .count();
        stats_panel_->submit("ui", ms);
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
}
} // namespace ivx
