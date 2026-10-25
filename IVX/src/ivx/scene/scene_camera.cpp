#include "ivxpch.h"
#include "scene_camera.h"

#include "ivx/core/base.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {
namespace {
// Reverse-Z projections for a right-handed view space (camera looks down -Z):
// near -> depth 1, far -> depth 0, matching glClipControl(GL_ZERO_TO_ONE) plus
// glDepthFunc(GL_GREATER).
//
// `n`/`f` are positive distances; the near plane lands at view z = -n.
static glm::mat4 ortho_rh_reverse_z(float left,
                                    float right,
                                    float bottom,
                                    float top,
                                    float z_near,
                                    float z_far) {
    glm::mat4 proj(1.f);
    proj[0][0] = 2.f / (right - left);
    proj[1][1] = 2.f / (top - bottom);
    proj[2][2] = 1.f / (z_far - z_near);
    proj[3][0] = -(right + left) / (right - left);
    proj[3][1] = -(top + bottom) / (top - bottom);
    proj[3][2] = z_far / (z_far - z_near);
    return proj;
}

// `n`/`f` are positive distances; the near plane lands at view z = -n.
static glm::mat4
perspective_rh_reverse_z(float fovY, float aspect, float n, float f) {
    float focal = 1.f / std::tan(fovY * 0.5f);

    glm::mat4 proj(0.f);
    proj[0][0] = focal / aspect;
    proj[1][1] = focal;
    proj[2][2] = n / (f - n);
    proj[2][3] = -1.f; // clip.w = -z_view: positive in front of the camera
    proj[3][2] = n * f / (f - n);
    return proj;
}

// Near/far are distances: keep them positive and ordered.
void validate_clip_planes(float &near_clip, float &far_clip) {
    near_clip = glm::max(near_clip, 0.001f);
    if (far_clip <= near_clip) {
        far_clip = near_clip + 0.001f;
    }
}
} // namespace

void SceneCamera::set_projection_type(ProjectionType type) {
    projection_type_ = type;
}

void SceneCamera::set_orthographic(float size,
                                   float near_clip,
                                   float far_clip) {
    validate_clip_planes(near_clip, far_clip);
    orthographic_size_ = size;
    orthographic_near_ = near_clip;
    orthographic_far_ = far_clip;
}

void SceneCamera::set_perspective(float vertical_fov,
                                  float near_clip,
                                  float far_clip) {
    validate_clip_planes(near_clip, far_clip);
    perspective_vertical_fov_ = vertical_fov;
    perspective_near_ = near_clip;
    perspective_far_ = far_clip;
}

void SceneCamera::set_viewport_size(uint32_t width, uint32_t height) {
    if (height == 0) {
        return;
    }
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);
}

void SceneCamera::set_orthographic_size(float size) {
    orthographic_size_ = size;
}

void SceneCamera::set_orthographic_near(float near_clip) {
    orthographic_near_ = near_clip;
}

void SceneCamera::set_orthographic_far(float far_clip) {
    orthographic_far_ = far_clip;
}

void SceneCamera::set_perspective_vertical_fov(float vertical_fov) {
    perspective_vertical_fov_ = vertical_fov;
}

void SceneCamera::set_perspective_near(float near_clip) {
    perspective_near_ = near_clip;
}

void SceneCamera::set_perspective_far(float far_clip) {
    perspective_far_ = far_clip;
}

float SceneCamera::get_orthographic_width() const {
    return orthographic_size_ * aspect_ratio_;
}

float SceneCamera::get_orthographic_height() const {
    return orthographic_size_;
}

glm::mat4 SceneCamera::get_proj_matrix() const {
    if (projection_type_ == ProjectionType::Orthographic) {
        const float top = orthographic_size_ * 0.5f;
        const float right = top * aspect_ratio_;
        // Distances -> view-space z (the camera looks down -Z).
        return ortho_rh_reverse_z(
            -right, right, -top, top, orthographic_near_, orthographic_far_);
    }

    return perspective_rh_reverse_z(perspective_vertical_fov_,
                                    aspect_ratio_,
                                    perspective_near_,
                                    perspective_far_);
}

} // namespace ivx
