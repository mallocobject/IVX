#include "edit_layer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/application.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/renderer/render_command.h"
#include "ivx/scene/scene.h"
#include "ivx/scene/scriptable_entity.h"
#include "scripts/controller.h"
#include <algorithm>
#include <cstdint>
#include <imgui.h>

namespace ivx {

static constexpr uint32_t WIDTH = 1280;
static constexpr uint32_t HEIGHT = 720;

EditLayer::EditLayer() : Layer("EditLayer") {
}

void EditLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig cfg;
    // cfg.PixelSnapH = true;
    ImFont *font =
        io.Fonts->AddFontFromFileTTF(FONTS "/consola.ttf", 16.0f, &cfg);
    IVX_CORE_ASSERT(font, "Failed to load Consolas");
    io.FontDefault = font;

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

    square_entity_.get_component<TransformComponent>().translation =
        glm::vec3{0.f};
    square_entity_.get_component<TransformComponent>().rotation = 0.f;
    square_entity_.get_component<TransformComponent>().scale =
        glm::vec2{400.f, 200.f};

    square_entity_.add_component<SpriteRendererComponent>(
        sub_texture_, 1.f, glm::vec4{1.f, 1.f, 1.f, 1.f});

    camera_entity_ = scene_->create_entity("Camera Entity");

    camera_entity_.add_component<CameraComponent>(
        static_cast<float>(HEIGHT), -1.f, 1.f, 1.f, true, false);
    camera_entity_.add_component<NativeScriptComponent>().bind<Controller>();

    second_camera_entity_ = scene_->create_entity("CameraB Entity");
    second_camera_entity_.add_component<CameraComponent>(
        static_cast<float>(HEIGHT) / 2.f, -1.f, 1.f, 1.f, false, true);

    setup_panels();
}

void EditLayer::on_detach() {
    IVX_PROFILE_FUNCTION();
}

void EditLayer::on_update(float dt) {
    IVX_PROFILE_FUNCTION();

    if (visible_) {
        const auto &spec = frame_buffer_->get_specification();
        if (spec.width != viewport_size_.x || spec.height != viewport_size_.y) {
            uint32_t w = static_cast<uint32_t>(viewport_size_.x);
            uint32_t h = static_cast<uint32_t>(viewport_size_.y);

            scene_->on_viewport_resize(w, h);
            frame_buffer_->resize(w, h);
        }
    }

    // update
    if (viewport_focused_) {
        camera_entity_.get_component<NativeScriptComponent>().enabled = true;
    } else {
        camera_entity_.get_component<NativeScriptComponent>().enabled = false;
    }

    scene_->on_update(dt);
    square_entity_.get_component<SpriteRendererComponent>().color = color_;

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
}

void EditLayer::on_event(Event &event) {
    if (event.is_in_category(EventCategory::kEventCategoryMouse |
                             EventCategory::kEventCategoryKeyboard)) {
        // IVX_VERBOSE("{}", event.to_string());
    }
}

void EditLayer::setup_panels() {
    IVX_PROFILE_FUNCTION();

    // Content panels first (they only emit widgets), then the hierarchy panel
    // so the draw order is deterministic: Hierarchy -> Viewport -> Settings.
    scene_hierarchy_panel_ = &panels_.add<SceneHierarchyPanel>();
    scene_hierarchy_panel_->set_scene_context(scene_);

    auto &viewport = panels_.add<LambdaPanel>(
        "Viewport", [this]() { draw_viewport_content(); });
    viewport.set_pre_begin([]() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
    });
    viewport.set_post_end([]() { ImGui::PopStyleVar(); });
    viewport.set_on_hidden([this]() {
        visible_ = false;
        viewport_focused_ = false;
        viewport_hovered_ = false;
        Application::get().imgui_layer()->block_event(true);
    });

    panels_.add<LambdaPanel>(
        "Settings", [this]() { draw_settings_content(); });
}

void EditLayer::on_render() {
    IVX_PROFILE_FUNCTION();

    // replace fullscreen host window + DockSpace
    ImGui::DockSpaceOverViewport(
        0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                Application::get().close();
            }
            ImGui::EndMenu();
        }
        panels_.draw_view_menu();
        ImGui::EndMainMenuBar();
    }

    panels_.on_render();
}

void EditLayer::draw_settings_content() {
    auto stats = Renderer2D::get_stats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.num_draw_calls);
    ImGui::Text("Quads: %d", stats.num_quads);
    ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
    ImGui::Text("Indices: %d", stats.get_total_index_count());

    if (square_entity_) {
        ImGui::Separator();
        auto &tag = square_entity_.get_component<TagComponent>().tag;
        ImGui::Text("%s", tag.c_str());

        ImGui::ColorEdit4("Square Color", glm::value_ptr(color_));
        ImGui::Separator();
    }

    static float zoom_level = 1.f;
    ImGui::DragFloat("Zoom", &zoom_level, 0.1f, 0.5f, 3.0f);

    if (primary_camera_) {
        ImGui::DragFloat2(
            "Camera Translation",
            glm::value_ptr(camera_entity_.get_component<TransformComponent>()
                               .translation));

        camera_entity_.get_component<CameraComponent>().camera.set_zoom_level(
            zoom_level);
    } else {
        ImGui::DragFloat2(
            "Camera Translation",
            glm::value_ptr(
                second_camera_entity_.get_component<TransformComponent>()
                    .translation));

        second_camera_entity_.get_component<CameraComponent>()
            .camera.set_zoom_level(zoom_level);
    }

    if (ImGui::Checkbox("Camera A", &primary_camera_)) {
        camera_entity_.get_component<CameraComponent>().primary =
            primary_camera_;
        second_camera_entity_.get_component<CameraComponent>().primary =
            !primary_camera_;
    }
}

void EditLayer::draw_viewport_content() {
    viewport_focused_ = ImGui::IsWindowFocused();
    viewport_hovered_ = ImGui::IsWindowHovered();
    Application::get().imgui_layer()->block_event(!viewport_focused_ ||
                                                  !viewport_hovered_);

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
