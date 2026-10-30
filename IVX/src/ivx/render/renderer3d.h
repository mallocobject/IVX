#pragma once

#include "camera.h"
#include "ivx/core/ref.hpp"
#include "mesh.h"
#include "shader.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

class Renderer3D {
  public:
    static void init();
    static void shutdown();

    // Submission scope of the current pass. Named like Renderer2D::begin_batch
    // so both renderers read the same way; 3D submits immediately today, so no
    // batch is accumulated.
    static void begin_batch(const Camera &camera);
    static void end_batch();

    // Per object submit: binds the shader, uploads the per object uniforms and
    // draws. No batching yet, which keeps the path honest for arbitrary meshes
    // and materials.
    static void submit(const Ref<Mesh> &mesh,
                       const Ref<Shader> &shader,
                       const glm::mat4 &transform,
                       const glm::vec4 &tint = glm::vec4{1.f});

    // Uses the built in lit shader created in init().
    static void submit(const Ref<Mesh> &mesh,
                       const glm::mat4 &transform,
                       const glm::vec4 &color = glm::vec4{1.f});

    struct Statistics {
        uint32_t draw_calls{0};
        uint32_t triangles{0};
    };

    static void reset_stats();
    static const Statistics &get_stats();
};

} // namespace ivx
