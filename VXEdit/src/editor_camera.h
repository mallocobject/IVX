#pragma once

#include <algorithm>
#include <cstdint>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <ivx.h>

namespace ivx {

class Event;

class EditorCamera : public Camera {
  public:
    EditorCamera() = default;

    // Orbit/pan while Alt is held. Only the pose parameters change; the
    // matrices are rebuilt when they are next read.
    void on_update(float delta_time);

    void on_event(Event &event);

    void set_viewport_size(uint32_t width, uint32_t height);

    void set_distance(float distance) {
        distance_ = std::clamp(distance, 0.2f, 20000.f);
    }

    void focus(const glm::vec3 &point) {
        focal_point_ = point;
    }

    glm::mat4 get_view_matrix() const override;
    glm::mat4 get_proj_matrix() const override;
    glm::mat4 get_view_projection() const {
        return get_proj_matrix() * get_view_matrix();
    }

    glm::vec3 get_position() const {
        return focal_point_ - get_forward() * distance_;
    }

    glm::vec3 get_forward() const;
    glm::vec3 get_up() const;
    glm::vec3 get_right() const;

    // Radians, like every other angle in the engine (the inspector converts for
    // display).
    float get_pitch() const {
        return pitch_;
    }
    float get_yaw() const {
        return yaw_;
    }
    float get_distance() const {
        return distance_;
    }
    float get_fov() const {
        return fov_;
    }

  private:
    void orbit(const glm::vec2 &delta);
    void pan(const glm::vec2 &delta);
    void dolly(float delta);

    glm::quat orientation() const;

    // Panning follows the zoom level, so dragging feels the same whether the
    // scene fills the viewport or is a speck in the distance.
    float pan_speed() const {
        return 0.0016f * distance_;
    }

    float fov_{glm::radians(45.f)};
    float near_clip_{0.01f};
    float far_clip_{10000.f};
    // The aspect ratio is derived from this, never cached: the projection is
    // rebuilt lazily anyway, so a second copy would only have to be kept in
    // sync.
    glm::vec2 viewport_size_{1280.f, 720.f};

    glm::vec3 focal_point_{0.f};
    float distance_{866.03f}; // 500 * sqrt(3): the same three-quarter view the
                              // cube opens with
    // Radians. A positive pitch looks down; the initial pair aims the camera at
    // the origin from (500, 500, 500).
    float pitch_{glm::radians(35.264f)};
    float yaw_{glm::radians(-45.f)};

    // Mouse position of the previous frame, so Alt+drag can work with deltas
    // instead of absolute coordinates (the cursor is not captured while
    // dragging).
    glm::vec2 last_mouse_{0.f};

    glm::mat4 view_matrix_{1.f};
    glm::mat4 proj_matrix_{1.f};
};

} // namespace ivx
