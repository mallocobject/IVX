#include "sandbox2d.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/renderer/render_command.h"
#include <cstdint>
#include <imgui.h>

Sandbox2D::Sandbox2D() : ivx::Layer("Sandbox2D") {
    camera_controller_ =
        ivx::make_ref<ivx::OrthoGraphicCameraController>(1080, 720, 1.f, true);
}

void Sandbox2D::on_attach() {
    IVX_PROFILE_FUNCTION();

    texture_ = ivx::Texture2D::create(TEXTURES "Checkerboard.png");
}

void Sandbox2D::on_detach() {
    IVX_PROFILE_FUNCTION();
}

void Sandbox2D::on_update(float delta_time) {
    IVX_PROFILE_FUNCTION();

    camera_controller_->on_update(delta_time);

    {
        // clear
        IVX_PROFILE_SCOPE("Renderer prep");
        ivx::RenderCommand::set_clear_color(glm::vec4{0.1f, 0.1f, 0.1f, 1});
        ivx::RenderCommand::clear();
    }

    { // draw
        IVX_PROFILE_SCOPE("Renderer draw");

        ivx::Renderer2D::begin_scene(camera_controller_->get_camera());
        // ivx::Renderer2D::draw_rotated_quad(
        //     {-100.f, 0.0f}, {0.8f, 0.8f}, 45.f, {0.8f, 0.2f, 0.3f, 1.0f});
        ivx::Renderer2D::draw_quad(
            {0.f, 0.f}, {300.f, 300.f}, {0.2f, 0.3f, 0.8f, 1.0f});
        ivx::Renderer2D::draw_quad(
            {0.0f, 0.0f, -0.1f}, {500.f, 500.f}, texture_, 10.f);
        ivx::Renderer2D::end_scene();
    }
}

void Sandbox2D::on_event(ivx::Event &event) {
    camera_controller_->on_event(event);
}

void Sandbox2D::on_render() {
    IVX_PROFILE_FUNCTION();

    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Color palette", glm::value_ptr(color_));

    ImGui::End();
}