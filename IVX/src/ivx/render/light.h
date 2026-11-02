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

constexpr uint32_t k_max_lights = 8;

// World space form of one light, produced by Scene::collect_lights() every
// frame.
struct Light {
    LightType type{LightType::IVX_Directional};
    glm::vec3 position{0.f};
    glm::vec3 direction{0.f, -1.f, 0.f}; // the direction the light travels in
    glm::vec3 color{1.f};
    float intensity{1.f};
    float range{1000.f};
    float inner_cos{1.f};
    float outer_cos{0.f};
};

// Scene wide ambient/exposure plus every light the scene collected.
struct LightSet {
    glm::vec3 ambient{0.05f};
    float exposure{1.f};
    std::array<Light, k_max_lights> lights{};
    uint32_t count{0};
};

} // namespace ivx
