#include "ivxpch.h"
#include "camera_controller.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/geometric.hpp"
#include "ivx/core/input.h"
#include "ivx/event/application_event.h"
#include "ivx/event/mouse_event.h"
#include "ivx/renderer/camera.h"
#include <algorithm>

namespace ivx {
OrthoGraphicCameraController::OrthoGraphicCameraController(int width,
                                                           int height,
                                                           float zoom_level,
                                                           bool enable_rotation)
    : width_(width), height_(height), zoom_level_(zoom_level),
      enable_rotation_(enable_rotation) {
    camera_ = make_ref<OrthoGraphicCamera>(-zoom_level * width / 2,
                                           zoom_level * width / 2,
                                           -zoom_level * height / 2,
                                           zoom_level * height / 2);
}

void OrthoGraphicCameraController::on_update(float dt) {
    IVX_PROFILE_FUNCTION();

    static glm::vec3 postion{0.f};
    static float rotation = 0.f;
    static constexpr float speed = 500.f;
    static constexpr float delta = 180.f;

    // IVX_INFO("delta time: {}", dt);

    glm::vec2 movement{0.f};
    if (Input::is_key_pressed(IVX_KEY_W)) {
        movement.y++;
    }
    if (Input::is_key_pressed(IVX_KEY_S)) {
        movement.y--;
    }
    if (Input::is_key_pressed(IVX_KEY_A)) {
        movement.x--;
    }
    if (Input::is_key_pressed(IVX_KEY_D)) {
        movement.x++;
    }

    if (movement.x != 0 || movement.y != 0) {
        postion.x += glm::normalize(movement).x * speed * dt;
        postion.y += glm::normalize(movement).y * speed * dt;
    }

    if (enable_rotation_) {
        if (Input::is_key_pressed(IVX_KEY_Q)) {
            rotation += delta * dt;
        }
        if (Input::is_key_pressed(IVX_KEY_E)) {
            rotation -= delta * dt;
        }
    }

    camera_->set_position(postion);
    camera_->set_rotation(rotation);
}

void OrthoGraphicCameraController::on_event(Event &event) {
    IVX_PROFILE_FUNCTION();

    EventDispatcher dispatcher(event);
    dispatcher.dispatch([this](WindowResizeEvent &e) {
        IVX_PROFILE_FUNCTION();

        resize(e.get_width(), e.get_height());

        return false;
    });

    dispatcher.dispatch([this](MouseScrolledEvent &e) {
        IVX_PROFILE_FUNCTION();

        zoom_level_ -= e.get_y_offset() * 0.5f;
        zoom_level_ = std::max(zoom_level_, 0.25f);
        camera_->set_view_field(-zoom_level_ * width_ / 2,
                                zoom_level_ * width_ / 2,
                                -zoom_level_ * height_ / 2,
                                zoom_level_ * height_ / 2);
        return false;
    });
}

void OrthoGraphicCameraController::resize(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    camera_->set_view_field(-zoom_level_ * width_ / 2,
                            zoom_level_ * width_ / 2,
                            -zoom_level_ * height_ / 2,
                            zoom_level_ * height_ / 2);
}
} // namespace ivx
