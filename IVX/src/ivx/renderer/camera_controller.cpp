#include "ivxpch.h"
#include "camera_controller.h"
#include "ivx/core/input.h"
#include "ivx/core/key_codes.h"
#include "ivx/core/mouse_button_codes.h"
#include "ivx/event/application_event.h"
#include "ivx/event/mouse_event.h"
#include "ivx/renderer/camera.h"
#include <algorithm>

namespace ivx {
CameraController::CameraController(int width,
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

void CameraController::on_update(float delta_time) {
    static glm::vec3 postion{0.f};
    static float degrees = 0.f;
    static constexpr float speed = 5.f;
    static constexpr float delta = 180.f;

    // IVX_INFO("delta time: {}", delta_time);

    if (ivx::Input::is_key_pressed(IVX_KEY_W)) {
        postion.y += speed * delta_time;
    }
    if (ivx::Input::is_key_pressed(IVX_KEY_S)) {
        postion.y -= speed * delta_time;
    }
    if (ivx::Input::is_key_pressed(IVX_KEY_A)) {
        postion.x -= speed * delta_time;
    }
    if (ivx::Input::is_key_pressed(IVX_KEY_D)) {
        postion.x += speed * delta_time;
    }

    if (enable_rotation_) {
        if (ivx::Input::IsMouseButtonPressed(IVX_MOUSE_BUTTON_LEFT)) {
            degrees += delta * delta_time;
        }
        if (ivx::Input::IsMouseButtonPressed(IVX_MOUSE_BUTTON_RIGHT)) {
            degrees -= delta * delta_time;
        }
    }

    camera_->set_position(postion);
    camera_->set_rotation(degrees);
}

void CameraController::on_event(Event &event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch([this](WindowResizeEvent &e) {
        // float aspect_ratio = static_cast<float>(e.get_width()) /
        // e.get_height(); if (aspect_ratio < aspect_ratio_) {
        //     camera_->set_view_field(-zoom_level_,
        //                             zoom_level_,
        //                             -zoom_level_ / aspect_ratio,
        //                             zoom_level_ / aspect_ratio);
        // } else {
        //     camera_->set_view_field(-zoom_level_ * aspect_ratio,
        //                             zoom_level_ * aspect_ratio,
        //                             -zoom_level_,
        //                             zoom_level_);
        // }
        // aspect_ratio_ = aspect_ratio;
        width_ = e.get_width();
        height_ = e.get_height();
        camera_->set_view_field(-zoom_level_ * width_ / 2,
                                zoom_level_ * width_ / 2,
                                -zoom_level_ * height_ / 2,
                                zoom_level_ * height_ / 2);

        return false;
    });

    dispatcher.dispatch([this](MouseScrolledEvent &e) {
        zoom_level_ -= e.get_y_offset() * 0.5f;
        zoom_level_ = std::max(zoom_level_, 0.25f);
        camera_->set_view_field(-zoom_level_ * width_ / 2,
                                zoom_level_ * width_ / 2,
                                -zoom_level_ * height_ / 2,
                                zoom_level_ * height_ / 2);
        return false;
    });
}
} // namespace ivx
