#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

enum class LightType : uint8_t {
    IVX_Directional = 0,
    IVX_Point = 1,
    IVX_Spot = 2,
};

inline constexpr uint32_t k_max_lights = 8;

// The part of a light the user authors: shared by the component that stores it
// and by the resolved light that gets uploaded, so defaults and meanings live
// in exactly one place.
struct LightProperties {
    LightType type{LightType::IVX_Directional};
    glm::vec3 color{1.f};
    float intensity{1.f};

    // Physical falloff for point and spot lights:
    //   attenuation = 1 / (constant + linear * d + quadratic * d * d)
    // The coefficients carry the world unit: quadratic = 1 is true inverse
    // square for a world measured in metres, so a world measured in hundreds of
    // units needs a much smaller quadratic (or an intensity expressed in those
    // units) to stay visible.
    float constant{1.f};
    float linear{0.f};
    float quadratic{1.f};

    float inner_angle{20.f}; // degrees, spot only
    float outer_angle{30.f}; // degrees, spot only
    bool cast_shadows{false};
    float shadow_depth_bias{0.25f};
    float shadow_normal_bias{0.5f};
    float shadow_slope_bias{2.5f};
    float shadow_near{0.1f};
    float shadow_range{30.f};
};

// World space form of one light, produced by Scene::collect_lights() every
// frame.
struct Light {
    LightProperties properties;
    glm::vec3 position{0.f};
    glm::vec3 direction{0.f, -1.f, 0.f}; // the direction the light travels in
    float inner_cos{1.f};
    float outer_cos{0.f};
};

// Scene wide ambient plus every light the scene collected. Exposure is not
// here: it belongs to the tone mapping pass, which reads Environment::exposure
// directly.
struct LightSet {
    glm::vec3 ambient{0.05f};
    std::array<Light, k_max_lights> lights{};
    uint32_t count{0};
};

} // namespace ivx
