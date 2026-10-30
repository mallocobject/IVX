#include "ivxpch.h"
#include "mesh.h"

#include "ivx/core/logger.h"
#include "ivx/render/buffer.h"
#include <algorithm>
#include <limits>

namespace ivx {
namespace {
Ref<BufferLayout> vertex_3d_layout() {
    return Ref<BufferLayout>{
        new BufferLayout{{"a_Position", ShaderDataType::vec3, false},
                         {"a_Normal", ShaderDataType::vec3, false},
                         {"a_TexCoord", ShaderDataType::vec2, false}}};
}
} // namespace

Mesh::Mesh(std::vector<Vertex3D> vertices, const std::vector<uint32_t> &indices)
    : index_count_(static_cast<uint32_t>(indices.size())) {
    IVX_CORE_ASSERT(!vertices.empty() && !indices.empty(), "Mesh is empty");

    auto vertex_buffer = VertexBuffer::create(
        reinterpret_cast<float *>(vertices.data()),
        static_cast<uint32_t>(vertices.size() * sizeof(Vertex3D) /
                              sizeof(float)));
    vertex_buffer->set_buffer_layout(vertex_3d_layout());

    auto index_buffer =
        IndexBuffer::create(const_cast<uint32_t *>(indices.data()),
                            static_cast<uint32_t>(indices.size()));

    vertex_array_ = VertexArray::create();
    vertex_array_->add_vertex_buffer(vertex_buffer);
    vertex_array_->set_index_buffer(index_buffer);

    const float inf = std::numeric_limits<float>::max();
    bounds_.min = glm::vec3{inf};
    bounds_.max = glm::vec3{-inf};
    for (const Vertex3D &vertex : vertices) {
        bounds_.min = glm::min(bounds_.min, vertex.position);
        bounds_.max = glm::max(bounds_.max, vertex.position);
    }
}

Ref<Mesh> Mesh::create(const std::vector<Vertex3D> &vertices,
                       const std::vector<uint32_t> &indices) {
    // The constructor is private, so make_ref() cannot reach it.
    return Ref<Mesh>(new Mesh(vertices, indices));
}

Ref<Mesh> Mesh::create_cube(float size) {
    const float h = size * 0.5f;

    const glm::vec3 normals[6] = {{1.f, 0.f, 0.f},
                                  {-1.f, 0.f, 0.f},
                                  {0.f, 1.f, 0.f},
                                  {0.f, -1.f, 0.f},
                                  {0.f, 0.f, 1.f},
                                  {0.f, 0.f, -1.f}};

    // Corners per face, counter clockwise seen from outside so back face
    // culling keeps them.
    const glm::vec3 corners[6][4] = {
        {{h, -h, h}, {h, -h, -h}, {h, h, -h}, {h, h, h}},
        {{-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}},
        {{-h, h, h}, {h, h, h}, {h, h, -h}, {-h, h, -h}},
        {{-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}},
        {{-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}},
        {{h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}},
    };

    const glm::vec2 uvs[4] = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};

    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(24);
    indices.reserve(36);

    for (int face = 0; face < 6; ++face) {
        const auto base = static_cast<uint32_t>(vertices.size());
        for (int corner = 0; corner < 4; ++corner) {
            vertices.push_back(
                Vertex3D{corners[face][corner], normals[face], uvs[corner]});
        }
        indices.insert(
            indices.end(),
            {base + 0, base + 1, base + 2, base + 2, base + 3, base + 0});
    }

    return create(vertices, indices);
}

} // namespace ivx
