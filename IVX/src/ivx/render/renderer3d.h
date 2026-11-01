#pragma once

#include "ivx/core/ref.hpp"
#include "material.h"
#include "mesh.h"
#include "shader.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

class Renderer3D {
  public:
    static void init();
    static void shutdown();

    // Submission scope of the current pass; 3D submits immediately, so nothing
    // is accumulated.
    static void begin_batch();
    static void end_batch();

    // Binds the material's shader (or the built in one), uploads the per object
    // uniforms and draws. No batching yet, which keeps the path honest for
    // arbitrary meshes and materials.
    static void submit(const Ref<Mesh> &mesh,
                       const Material &material,
                       const glm::mat4 &transform,
                       uint32_t uid = 0);

    struct Statistics {
        uint32_t draw_calls{0};
        uint32_t triangles{0};
    };

    static void reset_stats();
    static const Statistics &get_stats();
};

} // namespace ivx
