#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

// Camera parameters, modelled after Hazel's SceneCamera: the projection type is
// an explicit switch and each type keeps its own parameters, so flipping the
// type never destroys the other side's values.
//
// The engine renders with reverse-Z and glClipControl(GL_ZERO_TO_ONE), therefore
// the orthographic near/far are view-space z values (default -1 .. 1) instead of
// positive distances.
class SceneCamera {
  public:
    enum class ProjectionType { Perspective = 0, Orthographic = 1 };

    SceneCamera() = default;
    virtual ~SceneCamera() = default;

    void set_projection_type(ProjectionType type);
    ProjectionType get_projection_type() const {
        return projection_type_;
    }

    // Convenience setters that also select the matching projection type.
    void set_orthographic(float size, float near_clip, float far_clip);
    void set_perspective(float vertical_fov, float near_clip, float far_clip);

    // Aspect ratio follows the render target unless the camera is fixed.
    void set_viewport_size(uint32_t width, uint32_t height);
    float get_aspect_ratio() const {
        return aspect_ratio_;
    }

    // ---- orthographic ----
    float get_orthographic_size() const {
        return orthographic_size_;
    }
    void set_orthographic_size(float size);
    float get_orthographic_near() const {
        return orthographic_near_;
    }
    void set_orthographic_near(float near_clip);
    float get_orthographic_far() const {
        return orthographic_far_;
    }
    void set_orthographic_far(float far_clip);

    // ---- perspective ----
    float get_perspective_vertical_fov() const { // radians
        return perspective_vertical_fov_;
    }
    void set_perspective_vertical_fov(float vertical_fov);
    float get_perspective_near() const {
        return perspective_near_;
    }
    void set_perspective_near(float near_clip);
    float get_perspective_far() const {
        return perspective_far_;
    }
    void set_perspective_far(float far_clip);

    // Size of the visible area in world units (orthographic only).
    float get_orthographic_width() const;
    float get_orthographic_height() const;

    glm::mat4 get_proj_matrix() const;

  private:
    ProjectionType projection_type_{ProjectionType::Orthographic};

    float orthographic_size_{720.f};
    float orthographic_near_{-1.f};
    float orthographic_far_{1.f};

    float perspective_vertical_fov_{0.785398f}; // 45 degrees
    float perspective_near_{0.01f};
    float perspective_far_{1000.f};

    float aspect_ratio_{16.f / 9.f};
};

} // namespace ivx
