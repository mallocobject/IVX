#pragma once

#include <cmath>
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

// Rotation part of a transform with the scale divided out: the columns are
// normalised, so a matrix that also carries a scale still yields a usable
// rotation.
inline glm::mat3 rotation_of(const glm::mat4 &transform) {
    glm::mat3 rotation{transform};
    for (int column = 0; column < 3; column++) {
        const float length = glm::length(rotation[column]);
        rotation[column] /= length > 0.f ? length : 1.f;
    }
    return rotation;
}

// pitch/yaw/roll of a rotation matrix, in the yaw*pitch*roll order that
// detail::build_rotation() builds. Both Euler triples of the same rotation are
// possible (see nearest_equivalent_euler); this returns whichever the signs
// select.
inline glm::vec3 euler_from_rotation_matrix(const glm::mat3 &rotation) {
    const float pitch = std::asin(std::clamp(-rotation[2][1], -1.f, 1.f));
    const float yaw = std::atan2(rotation[2][0], rotation[2][2]);
    const float roll = std::atan2(rotation[0][1], rotation[1][1]);
    return glm::vec3{pitch, yaw, roll};
}

inline void decompose_transform(const glm::mat4 &transform,
                                glm::vec3 &translation,
                                glm::vec3 &rotation,
                                glm::vec3 &scale) {
    translation = glm::vec3(transform[3]);

    for (int column = 0; column < 3; column++) {
        const float length = glm::length(glm::vec3{transform[column]});
        scale[column] = length > 0.f ? length : 1.f;
    }

    rotation = euler_from_rotation_matrix(rotation_of(transform));
}

inline float wrap_angle(float radians) {
    return std::remainder(radians, 2.f * glm::pi<float>());
}

inline glm::vec3 nearest_equivalent_euler(const glm::vec3 &candidate,
                                          const glm::vec3 &previous) {
    const auto distance = [](const glm::vec3 &a, const glm::vec3 &b) {
        const glm::vec3 delta = a - b;
        return std::abs(wrap_angle(delta.x)) + std::abs(wrap_angle(delta.y)) +
               std::abs(wrap_angle(delta.z));
    };

    const glm::vec3 dual{glm::pi<float>() - candidate.x,
                         candidate.y + glm::pi<float>(),
                         candidate.z + glm::pi<float>()};
    return distance(candidate, previous) <= distance(dual, previous) ? candidate
                                                                     : dual;
}

inline glm::vec3 unwrap_euler(const glm::vec3 &angles,
                              const glm::vec3 &previous) {
    glm::vec3 result = angles;
    for (int axis = 0; axis < 3; axis++) {
        result[axis] =
            previous[axis] + wrap_angle(angles[axis] - previous[axis]);
    }
    return result;
}
} // namespace ivx
