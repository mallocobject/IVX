#include "edit_layer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/core/application.h"
#include "ivx/core/logger.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/renderer/render_command.h"
#include "ivx/scene/scene.h"
#include <algorithm>
#include <cfloat>
#include <cstdint>
#include <imgui.h>

namespace ivx {
EditLayer::EditLayer() : Layer("EditLayer") {
    camera_controller_ = make_ref<OrthoGraphicCameraController>(
        Application::get().get_window().get_width(),
        Application::get().get_window().get_height(),
        1.f,
        true);
}

void EditLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    texture_ = Texture2D::create(TEXTURES "tilemap.png");
    sub_texture_ =
        SubTexture2D::create(texture_, {2.f, 1.f}, {16.f, 16.f}, {2.f, 1.f});

    FrameBufferSpecification spec;
    spec.width = Application::get().get_window().get_width();
    spec.height = Application::get().get_window().get_height();

    frame_buffer_ = FrameBuffer::create(spec);

    scene_ = make_ref<Scene>();
    entity_ = scene_->create_entity("Green Square");

    auto transform = [](const glm::vec3 &translation = glm::vec3{0.f},
                        float rotation = 0.f,
                        const glm::vec2 &scale = glm::vec2{1.f}) -> glm::mat4 {
        return glm::translate(glm::mat4{1.f}, translation) *
               glm::rotate(glm::mat4{1.f},
                           glm::radians(rotation),
                           glm::vec3{0.f, 0.f, 1.f}) *
               glm::scale(glm::mat4{1.f}, glm::vec3{scale, 1.f});
    };

    entity_.get_component<TransformComponent>().transform =
        transform(glm::vec3{0.f}, 0.f, glm::vec2{400.f, 200.f});

    entity_.add_component<SpriteRendererComponent>(
        glm::vec4{1.f, 1.f, 1.f, 1.f}, nullptr, sub_texture_);

    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig cfg;
    // cfg.PixelSnapH = true;
    ImFont *font =
        io.Fonts->AddFontFromFileTTF(FONTS "/consola.ttf", 16.0f, &cfg);
    IVX_CORE_ASSERT(font, "Failed to load Consolas");
    io.FontDefault = font;
}

void EditLayer::on_detach() {
    IVX_PROFILE_FUNCTION();
}

void EditLayer::on_update(float delta_time) {
    IVX_PROFILE_FUNCTION();

    if (visible_) {
        const auto &spec = frame_buffer_->get_specification();
        if (spec.width != viewport_size_.x || spec.height != viewport_size_.y) {
            camera_controller_->resize(static_cast<uint32_t>(viewport_size_.x),
                                       static_cast<uint32_t>(viewport_size_.y));
            frame_buffer_->resize(static_cast<uint32_t>(viewport_size_.x),
                                  static_cast<uint32_t>(viewport_size_.y));
        }
    }

    // update
    if (viewport_focused_) {
        camera_controller_->on_update(delta_time);
    }

    scene_->on_update(delta_time);
    entity_.get_component<SpriteRendererComponent>().color = color_;

    // render
    Renderer2D::reset_stats();

    frame_buffer_->bind();

    {
        // clear
        IVX_PROFILE_SCOPE("Renderer prep");
        RenderCommand::set_clear_color(glm::vec4{0.1f, 0.1f, 0.1f, 1});
        RenderCommand::clear();
    }

    {
        // draw
        IVX_PROFILE_SCOPE("Renderer draw");

        Renderer2D::begin_scene(camera_controller_->get_camera());
        scene_->on_render();
        Renderer2D::end_scene();
    }

    frame_buffer_->unbind();
}

void EditLayer::on_event(Event &event) {
    if (event.is_in_category(EventCategory::kEventCategoryMouse |
                             EventCategory::kEventCategoryKeyboard)) {
        // IVX_VERBOSE("{}", event.to_string());
    }

    camera_controller_->on_event(event);
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
        ImGui::EndMainMenuBar();
    }

    draw_settings_panel();
}

void EditLayer::draw_settings_panel() {
    ImGui::Begin("Settings");

    auto stats = Renderer2D::get_stats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.num_draw_calls);
    ImGui::Text("Quads: %d", stats.num_quads);
    ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
    ImGui::Text("Indices: %d", stats.get_total_index_count());

    if (entity_) {
        ImGui::Separator();
        auto &tag = entity_.get_component<TagComponent>().value;
        ImGui::Text("%s", tag.c_str());

        auto &squareColor =
            entity_.get_component<SpriteRendererComponent>().color;
        ImGui::ColorEdit4("Square Color", glm::value_ptr(color_));
        ImGui::Separator();
    }

    ImGui::End();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 0.f});
    ImGui::Begin("Viewport");
    viewport_focused_ = ImGui::IsWindowFocused();
    viewport_hovered_ = ImGui::IsWindowHovered();
    Application::get().imgui_layer()->block_event(!viewport_focused_ ||
                                                  !viewport_hovered_);

    visible_ = ImGui::IsWindowCollapsed() == false;
    if (visible_) {
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

    ImGui::End();
    ImGui::PopStyleVar();
}
} // namespace ivx