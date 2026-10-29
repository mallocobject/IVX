#pragma once

#include "ivx/render/camera.h"
#include <cstdint>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {

class SceneCamera : public Camera {
  public:
    enum class ProjectionType { Perspective = 0, Orthographic = 1 };

    SceneCamera() = default;

    void set_projection_type(ProjectionType type);
    ProjectionType get_projection_type() const {
        return projection_type_;
    }

    // Convenience setters that also select the matching projection type.
    // near_clip/far_clip are positive distances.
    void set_orthographic(float size, float near_clip, float far_clip);
    void set_perspective(float vertical_fov, float near_clip, float far_clip);

    // Aspect ratio follows the render target unless the camera is fixed.
    void set_viewport_size(uint32_t width, uint32_t height);
    float get_aspect_ratio() const {
        return aspect_ratio_;
    }

    // ---- orthographic ----
    // Trivial setters are inline on purpose: script modules never link the
    // engine, so anything they are expected to call has to be header-only.
    float get_orthographic_size() const {
        return orthographic_size_;
    }
    void set_orthographic_size(float size) {
        orthographic_size_ = size;
    }
    float get_orthographic_near() const { // positive distance
        return orthographic_near_;
    }
    void set_orthographic_near(float near_clip) {
        orthographic_near_ = near_clip;
    }
    float get_orthographic_far() const { // positive distance
        return orthographic_far_;
    }
    void set_orthographic_far(float far_clip) {
        orthographic_far_ = far_clip;
    }

    // ---- perspective ----
    float get_perspective_vertical_fov() const { // radians
        return perspective_vertical_fov_;
    }
    void set_perspective_vertical_fov(float vertical_fov) { // radians
        perspective_vertical_fov_ = vertical_fov;
    }
    float get_perspective_near() const { // positive distance
        return perspective_near_;
    }
    void set_perspective_near(float near_clip) {
        perspective_near_ = near_clip;
    }
    float get_perspective_far() const { // positive distance
        return perspective_far_;
    }
    void set_perspective_far(float far_clip) {
        perspective_far_ = far_clip;
    }

    // Size of the visible area in world units (orthographic only).
    float get_orthographic_width() const;
    float get_orthographic_height() const;

    virtual glm::mat4 get_proj_matrix() const override;

  private:
    ProjectionType projection_type_{ProjectionType::Orthographic};

    float orthographic_size_{720.f};
    float orthographic_near_{0.01f};
    float orthographic_far_{1000.f};

    float perspective_vertical_fov_{glm::radians(90.f)};
    float perspective_near_{0.01f};
    float perspective_far_{10000.f};

    float aspect_ratio_{16.f / 9.f};
};

} // namespace ivx
