#include "sandbox2d.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/event.h"
#include "ivx/renderer/render_command.h"
#include <imgui.h>

Sandbox2D::Sandbox2D() : ivx::Layer("Sandbox2D") {
    camera_controller_ = ivx::make_ref<ivx::OrthoGraphicCameraController>(
        ivx::Application::get().get_window().get_width(),
        ivx::Application::get().get_window().get_height(),
        1.f,
        true);
}

void Sandbox2D::on_attach() {
    IVX_PROFILE_FUNCTION();

    texture_ = ivx::Texture2D::create(TEXTURES "tilemap.png");
    sub_texture_ = ivx::SubTexture2D::create(
        texture_, {2.f, 1.f}, {16.f, 16.f}, {2.f, 1.f});
}

void Sandbox2D::on_detach() {
    IVX_PROFILE_FUNCTION();
}

void Sandbox2D::on_update(float delta_time) {
    IVX_PROFILE_FUNCTION();

    // update
    camera_controller_->on_update(delta_time);

    ivx::Renderer2D::reset_stats();
}

void Sandbox2D::on_event(ivx::Event &event) {
    if (event.is_in_category(ivx::EventCategory::kEventCategoryMouse |
                             ivx::EventCategory::kEventCategoryKeyboard)) {
        // IVX_VERBOSE("{}", event.to_string());
    }

    camera_controller_->on_event(event);
}

void Sandbox2D::on_render() {
    IVX_PROFILE_FUNCTION();

    // Application::run() clears the default framebuffer right after on_update(),
    // so drawing has to happen here: anything issued in on_update() is wiped
    // before the buffer is presented.
    {
        IVX_PROFILE_SCOPE("Renderer draw");

        ivx::Renderer2D::begin_scene(camera_controller_->get_camera());
        ivx::Renderer2D::draw_quad({0.f, 0.f}, {600.f, 300.f}, sub_texture_);
        ivx::Renderer2D::end_scene();
    }

    draw_settings_panel();
}

void Sandbox2D::draw_settings_panel() {
    ImGui::Begin("Settings");

    auto stats = ivx::Renderer2D::get_stats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.num_draw_calls);
    ImGui::Text("Quads: %d", stats.num_quads);
    ImGui::Text("Vertices: %d", stats.get_total_vertex_count());
    ImGui::Text("Indices: %d", stats.get_total_index_count());

    ImGui::ColorEdit4("Square Color", glm::value_ptr(color_));

    ImGui::End();
}