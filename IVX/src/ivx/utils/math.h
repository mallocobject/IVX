#pragma once

#include <algorithm>
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

inline glm::mat4 rotation_matrix_from_euler(const glm::vec3 &rotation) {
    float yaw = rotation.y;
    float pitch = rotation.x;
    float roll = rotation.z;

    const float cy = std::cos(yaw), sy = std::sin(yaw);
    const float cp = std::cos(pitch), sp = std::sin(pitch);
    const float cr = std::cos(roll), sr = std::sin(roll);

    glm::mat4 R(1.0f);

    // GLM col-main: R[col][row]
    R[0][0] = cy * cr + sy * sp * sr;
    R[0][1] = cp * sr;
    R[0][2] = -sy * cr + cy * sp * sr;

    R[1][0] = -cy * sr + sy * sp * cr;
    R[1][1] = cp * cr;
    R[1][2] = sy * sr + cy * sp * cr;

    R[2][0] = sy * cp;
    R[2][1] = -sp;
    R[2][2] = cy * cp;

    return R;
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

inline float wrap_radians(float radians) {
    return std::remainder(radians, 2.f * glm::pi<float>());
}

inline float wrap_degrees(float degrees) {
    return std::remainder(degrees, 360.f);
}

inline glm::vec3 nearest_equivalent_euler(const glm::vec3 &candidate,
                                          const glm::vec3 &previous) {
    const auto distance = [](const glm::vec3 &a, const glm::vec3 &b) {
        const glm::vec3 delta = a - b;
        return std::abs(wrap_radians(delta.x)) +
               std::abs(wrap_radians(delta.y)) +
               std::abs(wrap_radians(delta.z));
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
            previous[axis] + wrap_radians(angles[axis] - previous[axis]);
    }
    return result;
}

// ---- display transfer -------------------------------------------------------
// Linear radiance to display values. ToneMappingPass mirrors these exact
// formulas in GLSL, so the editor preview, the smoke test reference and the GPU
// agree channel by channel.

// Reinhard: x / (1 + x). Cheap and monotonic, but it darkens mid tones and
// never reaches white.
inline glm::vec3 tonemap_reinhard(const glm::vec3 &linear) {
    return linear / (glm::vec3{1.f} + linear);
}

// Narkowicz's ACES approximation: filmic contrast with rolled off highlights.
inline glm::vec3 tonemap_aces(const glm::vec3 &linear) {
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;

    const glm::vec3 numerator = linear * (a * linear + b);
    const glm::vec3 denominator = linear * (c * linear + d) + e;
    return glm::clamp(numerator / denominator, 0.f, 1.f);
}

// Khronos PBR Neutral: leaves everything below 0.8 untouched and compresses
// only the top, so albedo colours stay recognisable. Highlights go towards
// white instead of clipping.
inline glm::vec3 tonemap_neutral(const glm::vec3 &linear) {
    constexpr float start_compression = 0.8f - 0.04f;
    constexpr float desaturation = 0.15f;

    glm::vec3 color = linear;
    const float darkest = glm::min(color.r, glm::min(color.g, color.b));
    // Straight section near black, constant offset above it (the official
    // curve).
    const float offset =
        darkest < 0.08f ? darkest - 6.25f * darkest * darkest : 0.04f;
    color -= glm::vec3{offset};

    const float peak = glm::max(color.r, glm::max(color.g, color.b));
    if (peak < start_compression) {
        return glm::max(color, glm::vec3{0.f});
    }

    const float d = 1.f - start_compression;
    const float new_peak = 1.f - d * d / (peak + d - start_compression);
    color *= new_peak / peak;

    const float whitepoint =
        1.f - 1.f / (desaturation * (peak - new_peak) + 1.f);
    return glm::max(glm::mix(color, glm::vec3{1.f}, whitepoint),
                    glm::vec3{0.f});
}

// Exact linear-to-sRGB transfer (IEC 61966-2-1), per channel.
inline float encode_srgb(float linear) {
    return linear <= 0.0031308f
               ? 12.92f * linear
               : 1.055f * std::pow(linear, 1.f / 2.4f) - 0.055f;
}

inline glm::vec3 encode_srgb(const glm::vec3 &linear) {
    return glm::vec3{
        encode_srgb(linear.r), encode_srgb(linear.g), encode_srgb(linear.b)};
}
} // namespace ivx
