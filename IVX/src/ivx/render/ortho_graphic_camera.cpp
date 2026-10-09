#include "ivxpch.h"
#include "ortho_graphic_camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"

namespace ivx {
OrthoGraphicCamera::OrthoGraphicCamera(float left,
                                       float right,
                                       float bottom,
                                       float top)
    : left_(left), right_(right), bottom_(bottom), top_(top) {
}

glm::mat4 OrthoGraphicCamera::get_view_matrix() const {
    auto transform = glm::translate(glm::mat4(1.f), position_) *
                     glm::rotate(glm::mat4(1.f),
                                 glm::radians(degrees_),
                                 glm::vec3(0.f, 0.f, 1.f));

    // view matrix
    return glm::inverse(transform);
}

glm::mat4 OrthoGraphicCamera::get_proj_matrix() const {
    // in opengl, the z from -1 to 1
    auto proj = glm::ortho(left_, right_, bottom_, top_, -1.f, 1.f);

    return proj;
}
} // namespace ivx