#include "ivxpch.h"
#include "scene_camera.h"

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
} // namespace

SceneCamera::SceneCamera(float left,
                         float right,
                         float bottom,
                         float top,
                         float n,
                         float f,
                         float zoom_level)
    : left_(left), right_(right), bottom_(bottom), top_(top), n_(n), f_(f),
      zoom_level_(zoom_level) {
}

void SceneCamera::set_view_field(
    float left, float right, float bottom, float top, float n, float f) {
    left_ = left;
    right_ = right;
    bottom_ = bottom;
    top_ = top;
    n_ = n;
    f_ = f;
}
void SceneCamera::set_zoom_level(float zoom_level) {
    zoom_level_ = zoom_level;
}

glm::mat4 SceneCamera::get_proj_matrix() const {
    return ortho_rh_reverse_z(left_ / zoom_level_,
                              right_ / zoom_level_,
                              bottom_ / zoom_level_,
                              top_ / zoom_level_,
                              n_,
                              f_);
}
} // namespace ivx