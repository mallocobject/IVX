#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace ivx {
// Reverse-Z projections for a right-handed view space (camera looks down -Z):
// near -> depth 1, far -> depth 0, matching glClipControl(GL_ZERO_TO_ONE) plus
// glDepthFunc(GL_GREATER).
//
// `n`/`f` are positive distances; the near plane lands at view z = -n.
inline glm::mat4 ortho_rh_reverse_z(float left,
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
inline glm::mat4
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
inline void validate_clip_planes(float &near_clip, float &far_clip) {
    near_clip = glm::max(near_clip, 0.001f);
    if (far_clip <= near_clip) {
        far_clip = near_clip + 0.001f;
    }
}

inline void decompose_transform(const glm::mat4 &transform,
                                glm::vec3 &translation,
                                glm::vec3 &rotation,
                                glm::vec3 &scale) {
    translation = glm::vec3(transform[3]);

    glm::mat3 rotation_mat{transform};

    for (int column = 0; column < 3; column++) {
        const float length = glm::length(rotation_mat[column]);
        scale[column] = length > 0.f ? length : 1.f;
        rotation_mat[column] /= scale[column];
    }

    const float pitch = std::asin(std::clamp(-rotation_mat[2][1], -1.f, 1.f));
    const float yaw = std::atan2(rotation_mat[2][0], rotation_mat[2][2]);
    const float roll = std::atan2(rotation_mat[0][1], rotation_mat[1][1]);
    rotation = glm::vec3{pitch, yaw, roll};
}
} // namespace ivx
