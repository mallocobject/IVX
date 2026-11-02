#pragma once

#include <glm/glm.hpp>

namespace ivx {

// Scene wide imaging settings: what has no position or direction of its own.
// Lights are entities and live in LightComponent.
struct Environment {
    glm::vec3 ambient{0.35f};
    float exposure{1.f};
};

} // namespace ivx