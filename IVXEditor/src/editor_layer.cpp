#include "editor_layer.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/renderer/render_command.h"
#include <imgui.h>

namespace ivx {
EditorLayer::EditorLayer() : Layer("EditorLayer") {
    camera_controller_ = make_ref<OrthoGraphicCameraController>(
        Application::get().get_window().get_width(),
        Application::get().get_window().get_height(),
        1.f,
        true);
}

void EditorLayer::on_attach() {
    IVX_PROFILE_FUNCTION();

    texture_ = Texture2D::create(TEXTURES "tilemap.png");
    sub_texture_ =
        SubTexture2D::create(texture_, {2.f, 1.f}, {16.f, 16.f}, {2.f, 1.f});

    FrameBufferSpecification spec;
    spec.width = Application::get().get_window().get_width();
    spec.height = Application::get().get_window().get_height();

    frame_buffer_ = FrameBuffer::create(spec);
}

void EditorLayer::on_detach() {
    IVX_PROFILE_FUNCTION();
}

void EditorLayer::on_update(float delta_time) {
    IVX_PROFILE_FUNCTION();

    // update
    camera_controller_->on_update(delta_time);

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
        // Renderer2D::draw_rotated_quad(
        //     {-100.f, 0.0f}, {400.f, 400.f}, 45.f, color_);
        Renderer2D::draw_quad({0.f, 0.f}, {600.f, 300.f}, sub_texture_);
        // Renderer2D::draw_quad(
        //     {0.0f, 0.0f, -0.1f}, {500.f, 500.f}, texture_, 10.f);
        Renderer2D::end_scene();

        // Renderer2D::begin_scene(camera_controller_->get_camera());
        // for (float y = -500.f; y < 500.f; y += 50.f) {
        //     for (float x = -500.f; x < 500.f; x += 50.f) {
        //         glm::vec4 color{
        //             (x + 500.f) / 1000.f, 0.4f, (y + 500.f) / 1000.f, 0.7f};
        //         Renderer2D::draw_quad({x, y}, {40.f, 40.f}, color);
        //     }
        // }
        // Renderer2D::end_scene();
    }

    frame_buffer_->unbind();
}

void EditorLayer::on_event(Event &event) {
    if (event.is_in_category(EventCategory::kEventCategoryMouse |
                             EventCategory::kEventCategoryKeyboard)) {
        // IVX_VERBOSE("{}", event.to_string());
    }

    camera_controller_->on_event(event);
}

void EditorLayer::on_render() {
    IVX_PROFILE_FUNCTION();

    static bool docking_enabled = true;
    if (docking_enabled) {
        // replace fullscreen host window + DockSpace
        ImGui::DockSpaceOverViewport(0,
                                     ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    Application::get().close();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    draw_settings_panel();
}

void EditorLayer::draw_settings_panel() {
    ImGui::Begin("Settings");

    auto stats = Renderer2D::get_stats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.num_draw_calls);
    ImGui::Text("Quads: %d", stats.num_quads);
    ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
    ImGui::Text("Indices: %d", stats.get_total_index_count());

    ImGui::ColorEdit4("Square Color", glm::value_ptr(color_));

    ImGui::Image(
        static_cast<ImTextureID>(frame_buffer_->get_color_attachment()),
        ImVec2{128.0f, 72.0f},
        ImVec2{0.0f, 1.0f},
        ImVec2{1.0f, 0.0f});

    ImGui::End();
}
} // namespace ivx