#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace ivx {

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

  private:
    Mesh(std::vector<Vertex3D> vertices, const std::vector<uint32_t> &indices);

    Ref<VertexArray> vertex_array_;
    Bounds bounds_;
    uint32_t index_count_{0};
};

} // namespace ivx
