#include "ivxpch.h"
#include "scene_camera.h"

#include "ivx/core/base.h"
#include "ivx/utils/math.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {
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
