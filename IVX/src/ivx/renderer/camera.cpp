#include "ivxpch.h"
#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"
#include <minwindef.h>

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

OrthoGraphicCamera::OrthoGraphicCamera(float left,
                                       float right,
                                       float bottom,
                                       float top)
    : left_(left), right_(right), bottom_(bottom), top_(top) {
}

glm::mat4 OrthoGraphicCamera::get_view_matrix() const {
    IVX_PROFILE_FUNCTION();

    auto transform = glm::translate(glm::mat4(1.f), position_) *
                     glm::rotate(glm::mat4(1.f),
                                 glm::radians(degrees_),
                                 glm::vec3(0.f, 0.f, 1.f));

    // view matrix
    return glm::inverse(transform);
}

glm::mat4 OrthoGraphicCamera::get_proj_matrix() const {
    IVX_PROFILE_FUNCTION();

    // in opengl, the z from -1 to 1
    // auto proj = glm::ortho(left_, right_, bottom_, top_, -1.f, 1.f);

    // reverse-z
    auto proj = ortho_rh_reverse_z(left_, right_, bottom_, top_, -1.f, 1.f);

    return proj;
}

PerspectiveCamera::PerspectiveCamera(const glm::vec3 &eye,
                                     const glm::vec3 &center,
                                     const glm::vec3 &up,
                                     float fov,
                                     int width,
                                     int height,
                                     float n,
                                     float f)
    : eye_(eye), center_(center), up_(up), fov_(fov), width_(width),
      height_(height), near_(n), far_(f) {
}

glm::mat4 PerspectiveCamera::get_view_matrix() const {
    IVX_PROFILE_FUNCTION();

    return glm::lookAtRH(eye_, center_, up_);
}

glm::mat4 PerspectiveCamera::get_proj_matrix() const {
    IVX_PROFILE_FUNCTION();

    // return glm::perspectiveFovLH_ZO(fov_,
    //                                 static_cast<float>(width_),
    //                                 static_cast<float>(height_),
    //                                 near_,
    //                                 far_);

    return perspective_rh_reverse_z(
        fov_, static_cast<float>(width_) / height_, near_, far_);
}
} // namespace ivx