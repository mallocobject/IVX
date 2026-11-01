#pragma once

#include "ivx/core/ref.hpp"
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

namespace ivx {

// Value semantics on purpose: editing one entity's material must not change
// every entity that shares the same shader.
struct Material {
    // nullptr = Renderer3D's built in mesh shader.
    Ref<Shader> shader;
    // nullptr = 1x1 white texture, so the shader never branches on "has a map".
    Ref<Texture2D> albedo_map;
    glm::vec4 albedo{1.f};
    float tiling_factor{1.f};
};
} // namespace ivx
