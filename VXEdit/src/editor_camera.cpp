#include "editor_camera.h"

#include "ivx/core/input.h"
#include "ivx/event/event.h"
#include "ivx/event/mouse_event.h"
#include "ivx/utils/math.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cmath>

namespace ivx {

void EditorCamera::set_viewport_size(uint32_t width, uint32_t height) {
    const glm::vec2 size{std::max(width, 1u), std::max(height, 1u)};
    if (size == viewport_size_) {
        return;
    }
}

void EditorCamera::on_update(float /*delta_time*/) {
    const glm::vec2 mouse = Input::get_mouse_pos();
    const glm::vec2 delta = mouse - last_mouse_;
    last_mouse_ = mouse;

    if (!Input::is_enabled() || !Input::is_key_pressed(KeyCode::LeftAlt)) {
        return;
    }

    if (Input::is_mouse_button_pressed(MouseCode::ButtonMiddle)) {
        pan(delta);
    } else if (Input::is_mouse_button_pressed(MouseCode::ButtonLeft)) {
        orbit(delta);
    }
}

void EditorCamera::on_event(Event &event) {
    EventDispatcher dispatcher{event};
    dispatcher.dispatch([this](MouseScrolledEvent &e) {
        dolly(e.get_y_offset());
        return false;
    });
}

glm::mat4 EditorCamera::get_view_matrix() const {
    auto position = get_position();

    const glm::mat4 pose = glm::translate(glm::mat4{1.f}, position) *
                           glm::mat4_cast(orientation());
    return glm::inverse(pose);
}

glm::mat4 EditorCamera::get_proj_matrix() const {
    const float aspect_ratio = viewport_size_.x / viewport_size_.y;

    // Reverse-Z, like SceneCamera: the renderer expects near -> 1, far -> 0
    // with GL_GREATER, so a plain glm::perspective() would draw nothing.
    return perspective_rh_reverse_z(fov_, aspect_ratio, near_clip_, far_clip_);
}

void EditorCamera::orbit(const glm::vec2 &delta) {
    constexpr float kSpeed = 0.005f;

    yaw_ += delta.x * kSpeed;
    pitch_ += delta.y * kSpeed;

    // Looking straight up or down would collapse the orbit axis and flip the
    // view.
    const float limit = glm::radians(89.f);
    pitch_ = std::clamp(pitch_, -limit, limit);
}

void EditorCamera::pan(const glm::vec2 &delta) {
    const float speed = pan_speed();
    focal_point_ += -get_right() * delta.x * speed;
    focal_point_ += get_up() * delta.y * speed;
}

void EditorCamera::dolly(float delta) {
    if (delta == 0.f) {
        return;
    }
    // Exponential instead of linear: one notch is always the same fraction of
    // the distance, so the zoom feels identical close up and far away.
    distance_ *= std::pow(0.9f, delta);
    distance_ = std::clamp(distance_, 0.2f, 20000.f);
}

glm::quat EditorCamera::orientation() const {
    return glm::quat{glm::vec3(-pitch_, -yaw_, 0.f)};
}

glm::vec3 EditorCamera::get_forward() const {
    return orientation() * glm::vec3{0.f, 0.f, -1.f};
}

glm::vec3 EditorCamera::get_up() const {
    return orientation() * glm::vec3{0.f, 1.f, 0.f};
}

glm::vec3 EditorCamera::get_right() const {
    return orientation() * glm::vec3{1.f, 0.f, 0.f};
}

} // namespace ivx
