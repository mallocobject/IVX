#pragma once

#include "ivx/utils/math.h"
#include <cstdint>
#include <string_view>

namespace ivx {

// Tone mapping curves, in the order the shader switches on them.
enum class ToneMappingCurve : uint8_t {
    IVX_Reinhard = 0,
    IVX_ACES = 1,
    IVX_Neutral = 2,
};

inline const char *tone_mapping_curve_name(ToneMappingCurve curve) {
    switch (curve) {
    case ToneMappingCurve::IVX_ACES:
        return "ACES";
    case ToneMappingCurve::IVX_Neutral:
        return "Khronos Neutral";
    case ToneMappingCurve::IVX_Reinhard:
    default:
        return "Reinhard";
    }
}

inline ToneMappingCurve tone_mapping_curve_from_name(std::string_view name) {
    if (name == "ACES") {
        return ToneMappingCurve::IVX_ACES;
    }
    if (name == "Khronos Neutral") {
        return ToneMappingCurve::IVX_Neutral;
    }
    return ToneMappingCurve::IVX_Reinhard;
}

// CPU mirror of the shader, so a preview or a test can predict the GPU output.
inline glm::vec3 tonemap(ToneMappingCurve curve, const glm::vec3 &linear) {
    switch (curve) {
    case ToneMappingCurve::IVX_ACES:
        return tonemap_aces(linear);
    case ToneMappingCurve::IVX_Neutral:
        return tonemap_neutral(linear);
    case ToneMappingCurve::IVX_Reinhard:
    default:
        return tonemap_reinhard(linear);
    }
}

} // namespace ivx
