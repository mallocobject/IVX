#include "ivxpch.h"
#include "scene_camera.h"

#include "ivx/core/base.h"
#include <cstddef>
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {
namespace {
static glm::mat4 ortho_rh_reverse_z(
    float left, float right, float bottom, float top, float n, float f) {
    glm::mat4 proj(1.f);
    proj[0][0] = 2.f / (right - left);
    proj[1][1] = 2.f / (top - bottom);
    proj[2][2] = 1.f / (f - n);
    proj[3][0] = -(right + left) / (right - left);
    proj[3][1] = -(top + bottom) / (top - bottom);
    proj[3][2] = f / (f - n);
    return proj;
}

static glm::mat4
perspective_rh_reverse_z(float fovY, float aspect, float n, float f) {
    float focal = 1.f / std::tan(fovY * 0.5f);

    glm::mat4 proj(0.f);
    proj[0][0] = focal / aspect;
    proj[1][1] = focal;
    proj[2][2] = n / (f - n);
    proj[2][3] = -1.f;
    proj[3][2] = n * f / (f - n);
    return proj;
}
} // namespace

SceneCamera::SceneCamera(float size, float n, float f, float zoom)
    : mode(Mode::Orthogonal), size_zoom_(size, zoom), n_(n), f_(f) {
}

SceneCamera::SceneCamera(float fov, float n, float f)
    : mode(Mode::Perspective), fov_(fov), n_(n), f_(f) {
}

void SceneCamera::set_viewport_size(uint32_t width, uint32_t height) {
    aspect_ratio_ = static_cast<float>(width) / height;
}

void SceneCamera::set_zoom_level(float zoom) {
    IVX_CORE_ASSERT(mode == Mode::Orthogonal,
                    "Perspective camera cannot change its zoom level!");

    size_zoom_.zoom = zoom;
}

void SceneCamera::set_fov(float fov) {
    IVX_CORE_ASSERT(mode == Mode::Perspective,
                    "Perspective camera cannot change its zoom level!");

    fov_ = fov;
}

glm::mat4 SceneCamera::get_proj_matrix() const {
    if (mode == Mode::Orthogonal) {
        float right = aspect_ratio_ * size_zoom_.size * 0.5f / size_zoom_.zoom;
        float top = size_zoom_.size * 0.5f / size_zoom_.zoom;
        float left = -right;
        float bottom = -top;

        return ortho_rh_reverse_z(left, right, bottom, top, n_, f_);
    } else {
        return perspective_rh_reverse_z(fov_, aspect_ratio_, n_, f_);
    }
}
} // namespace ivx