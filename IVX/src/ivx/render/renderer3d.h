#pragma once

#include "ivx/core/ref.hpp"
#include "material.h"
#include "mesh.h"
#include "shader.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

struct DepthDrawContext {
    enum class Encoding {
        Projected,      // directional, spot
        RadialReverseZ, // point
    };
    Encoding encoding{Encoding::Projected};
    glm::vec3 light_position{0.f};
    float near_plane{0.1f};
    float far_plane{30.f};
};

class Renderer3D {
  public:
    static void init();
    static void shutdown();

    // Submission scope of the current pass; 3D submits immediately, so nothing
    // is accumulated.
    static void begin();
    static void end();

    // Binds the material's shader (or the built in one), uploads the per object
    // uniforms and draws. No batching yet, which keeps the path honest for
    // arbitrary meshes and materials.
    static void submit(const Ref<Mesh> &mesh,
                       const Material &material,
                       const glm::mat4 &transform,
                       uint32_t uid = 0);

    // Depth-only submission for a shadow pass: the built in depth shader, no
    // material, no textures, and the same statistics as a normal draw.
    static void submit_depth(const Ref<Mesh> &mesh, const glm::mat4 &transform);
    static void begin_depth(const DepthDrawContext &context = {});
    static void end_depth();

    struct Statistics {
        uint32_t draw_calls{0};
        uint32_t triangles{0};
    };

    static void reset_stats();
    static const Statistics &get_stats();
};

} // namespace ivx
