#pragma once

#include "ivx/render/tone_mapping_curve.h"
#include <array>
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

inline constexpr std::array<uint32_t, 4> k_shadow_resolutions{
    512, 1024, 2048, 4096};

inline uint32_t shadow_resolution_or_default(uint32_t resolution) {
    for (uint32_t supported : k_shadow_resolutions) {
        if (resolution == supported)
            return resolution;
    }
    return 2048;
}

struct ShadowSettings {
    uint32_t resolution{2048};
    float distance{120.f};
};

struct GroundSettings {
    bool enabled{false};
    float height{0.f};
    float grid_spacing{1.f};
    glm::vec3 color{0.4f};
    glm::vec3 grid_color{0.2f};
};

// Scene wide imaging settings: what has no position or direction of its own.
// Lights are entities and live in LightComponent.
struct Environment {
    glm::vec3 ambient{0.15f};
    float exposure{1.f};
    ToneMappingCurve tonemap{ToneMappingCurve::IVX_Reinhard};
    ShadowSettings shadows{};
    GroundSettings ground{};
};

} // namespace ivx
