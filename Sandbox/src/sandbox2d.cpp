#include "sandbox2d.h"
#include <cstdint>
#include <imgui.h>

Sandbox2D::Sandbox2D() : ivx::Layer("Sandbox2D") {
}

void Sandbox2D::on_attach() {
    ivx::RenderCommand::set_clear_color(glm::vec4(0.f));
    texture_ = ivx::Texture2D::create(TEXTURES "Checkerboard.png");
    camera_controller_ =
        ivx::make_ref<ivx::OrthoGraphicCameraController>(1080, 720, 1.f, true);
}

void Sandbox2D::on_detach() {
}

void Sandbox2D::on_update(float delta_time) {
    PROFILE_SCOPE("Sandbox::on_update");

    {
        PROFILE_SCOPE("CameraController::on_update");
        camera_controller_->on_update(delta_time);
    }
    { // draw
        PROFILE_SCOPE("Renderer draw");

        ivx::Renderer2D::begin_scene(camera_controller_->get_camera());

        ivx::Renderer2D::draw_quad(
            {-100.f, 0.0f}, {0.8f, 0.8f}, {0.8f, 0.2f, 0.3f, 1.0f});
        ivx::Renderer2D::draw_quad(
            {50.f, -50.f}, {0.5f, 0.75f}, {0.2f, 0.3f, 0.8f, 1.0f});
        ivx::Renderer2D::draw_quad({0.0f, 0.0f, -0.1f}, {10.f, 10.f}, texture_);

        ivx::Renderer2D::end_scene();
    }
}

void Sandbox2D::on_event(ivx::Event &event) {
    camera_controller_->on_event(event);
}

void Sandbox2D::on_render() {
    ImGui::Begin("Settings");
    ImGui::ColorEdit4("Color palette", glm::value_ptr(color_));

    for (auto &pr : prs_) {
        ImGui::Text("%.3fms %.*s",
                    pr.duration,
                    static_cast<uint32_t>(pr.label.size()),
                    pr.label.data());
    }
    prs_.clear();

    ImGui::End();
}