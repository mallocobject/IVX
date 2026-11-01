#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace ivx {

// What a Mesh was built from. Scene files store this instead of vertex data, so
// only procedurally generated primitives survive a save/load round trip for
// now.
enum class MeshPrimitive : uint8_t {
    kCustom = 0,
    kCube,
};

struct Vertex3D {
    glm::vec3 position{0.f};
    glm::vec3 normal{0.f, 0.f, 1.f};
    glm::vec2 tex_coord{0.f};
};

struct Bounds {
    glm::vec3 min{0.f};
    glm::vec3 max{0.f};

    glm::vec3 center() const {
        return (min + max) * 0.5f;
    }
    glm::vec3 extent() const {
        return (max - min) * 0.5f;
    }
};

class Mesh : public RefCounted {
  public:
    static Ref<Mesh> create(const std::vector<Vertex3D> &vertices,
                            const std::vector<uint32_t> &indices);
    // Unit cube centred on the origin. 24 vertices so every face has its own
    // normal.
    static Ref<Mesh> create_cube(float size = 1.f);

    const Ref<VertexArray> &get_vertex_array() const {
        return vertex_array_;
    }
    const Bounds &get_bounds() const {
        return bounds_;
    }
    uint32_t get_index_count() const {
        return index_count_;
    }

    MeshPrimitive get_primitive() const {
        return primitive_;
    }
    // Only meaningful for primitives: the size the mesh was generated with.
    float get_primitive_size() const {
        return primitive_size_;
    }

  private:
    Mesh(std::vector<Vertex3D> vertices, const std::vector<uint32_t> &indices);

    Ref<VertexArray> vertex_array_;
    Bounds bounds_;
    uint32_t index_count_{0};
    MeshPrimitive primitive_{MeshPrimitive::kCustom};
    float primitive_size_{0.f};
};

} // namespace ivx
