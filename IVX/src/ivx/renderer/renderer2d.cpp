#include "ivxpch.h"
#include "renderer2d.h"
#include "buffer.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/trigonometric.hpp"
#include "ivx/renderer/renderer.h"
#include "ivx/renderer/shader.h"
#include "ivx/renderer/texture.h"
#include "ivx/renderer/vertex_array.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <ranges>

namespace ivx {
struct QuadVertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 tex_coord;
    int32_t tex_index;
    float tiling_factor;
};

struct Renderer2DData {
    static constexpr uint32_t max_num_quads = 10000;
    static constexpr uint32_t max_num_vertices = max_num_quads * 4;
    static constexpr uint32_t max_num_indices = max_num_quads * 6;
    static constexpr uint32_t max_num_tex_slots = 32;

    Ref<VertexArray> quad_vertex_array;
    Ref<VertexBuffer> quad_vertex_buffer;

    Ref<Texture2D> white_texture;
    Ref<Shader> texture_shader;

    uint32_t quad_index_count = 0;
    QuadVertex *quad_vertex_buffer_base = nullptr;
    QuadVertex *quad_vertex_buffer_ptr = nullptr;

    std::array<Ref<Texture2D>, max_num_tex_slots> tex_slots;
    int32_t tex_slot_index = 1; // 0 = white texture

    glm::vec4 quad_vertex_position[4];

    Renderer2D::Statistics stats;
};

static Renderer2DData data;

void Renderer2D::init() {
    IVX_PROFILE_FUNCTION();

    data.quad_vertex_array = VertexArray::create();
    data.quad_vertex_buffer = VertexBuffer::create(
        Renderer2DData::max_num_vertices * sizeof(QuadVertex));

    // float vertices[5 * 4] = {
    //     -300.f, -300.f, 0.f, 0.f, 0.f, 300.f,  -300.f, 0.f, 1.f, 0.f,
    //     300.f,  300.f,  0.f, 1.f, 1.f, -300.f, 300.f,  0.f, 0.f, 1.f,
    // };

    Ref<ivx::BufferLayout> bl{
        new BufferLayout{{"a_Position", ShaderDataType::vec3, false},
                         {"a_Color", ShaderDataType::vec4, false},
                         {"a_TexCoord", ShaderDataType::vec2, false},
                         {"a_TexIndex", ShaderDataType::ivec, false},
                         {"a_TilingFactor", ShaderDataType::vec, false}}};

    data.quad_vertex_buffer->set_buffer_layout(bl);

    data.quad_vertex_array->add_vertex_buffer(data.quad_vertex_buffer);

    data.quad_vertex_buffer_base =
        new QuadVertex[Renderer2DData::max_num_vertices];

    uint32_t *quad_indices = new uint32_t[Renderer2DData::max_num_indices];

    for (uint32_t offset = 0, i = 0; i < Renderer2DData::max_num_indices;
         i += 6, offset += 4) {
        quad_indices[i + 0] = offset + 0;
        quad_indices[i + 1] = offset + 1;
        quad_indices[i + 2] = offset + 2;

        quad_indices[i + 3] = offset + 0;
        quad_indices[i + 4] = offset + 2;
        quad_indices[i + 5] = offset + 3;
    }

    auto quad_ib = IndexBuffer::create(quad_indices, data.max_num_indices);
    delete[] quad_indices;

    data.quad_vertex_array->set_index_buffer(quad_ib);

    data.white_texture = Texture2D::create(1, 1);
    uint32_t white_texture_data = -1; // 0xffffffff
    data.white_texture->set_data(&white_texture_data, sizeof(uint32_t));
    data.tex_slots[0] = data.white_texture;

    int32_t samplers[Renderer2DData::max_num_tex_slots];
    for (uint32_t i = 0; i < Renderer2DData::max_num_tex_slots; i++) {
        samplers[i] = i;
    }

    data.texture_shader = Shader::create(SHADERS "texture.glsl");
    data.texture_shader->set(
        "u_Textures", samplers, Renderer2DData::max_num_tex_slots);

    data.quad_vertex_position[0] = {-0.5f, -0.5f, 0.f, 1.f};
    data.quad_vertex_position[1] = {0.5f, -0.5f, 0.f, 1.f};
    data.quad_vertex_position[2] = {0.5f, 0.5f, 0.f, 1.f};
    data.quad_vertex_position[3] = {-0.5f, 0.5f, 0.f, 1.f};
}

void Renderer2D::shutdown() {
    IVX_PROFILE_FUNCTION();
}

void Renderer2D::begin_scene(const Ref<OrthoGraphicCamera> &camera) {
    IVX_PROFILE_FUNCTION();

    auto vp = camera->get_proj_matrix() * camera->get_view_matrix();
    data.texture_shader->set("mvp", vp);

    data.quad_index_count = 0;
    data.quad_vertex_buffer_ptr = data.quad_vertex_buffer_base;

    data.tex_slot_index = 1;
}

void Renderer2D::end_scene() {
    IVX_PROFILE_FUNCTION();

    flush();
}

void Renderer2D::flush() {
    IVX_PROFILE_FUNCTION();

    if (data.quad_index_count == 0) {
        return;
    }

    const auto data_size = static_cast<uint32_t>(
        reinterpret_cast<uint8_t *>(data.quad_vertex_buffer_ptr) -
        reinterpret_cast<uint8_t *>(data.quad_vertex_buffer_base));
    data.quad_vertex_buffer->set_data(data.quad_vertex_buffer_base, data_size);

    for (int32_t i = 0; i < data.tex_slot_index; i++) {
        data.tex_slots[i]->bind(i);
    }

    data.quad_vertex_array->bind();
    data.texture_shader->bind();
    RenderCommand::draw(data.quad_vertex_array, data.quad_index_count);

    data.quad_index_count = 0;
    data.quad_vertex_buffer_ptr = data.quad_vertex_buffer_base;
    data.tex_slot_index = 1;

    data.stats.num_draw_calls++;
}

void Renderer2D::draw_quad(const glm::vec2 &position,
                           const glm::vec2 &size,
                           const glm::vec4 &color) {
    draw_quad(glm::vec3{position, 0.f}, size, color);
}

void Renderer2D::draw_quad(const glm::vec3 &position,
                           const glm::vec2 &size,
                           const glm::vec4 &color) {
    IVX_PROFILE_FUNCTION();

    if (data.quad_index_count >= Renderer2DData::max_num_indices) {
        flush();
    }

    constexpr int tex_index = 0;
    constexpr float tiling_factor = 1.f;
    constexpr glm::vec2 tex_coords[]{
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};

    glm::mat4 transform = glm::translate(glm::mat4{1.f}, position) *
                          glm::scale(glm::mat4{1.f}, {size.x, size.y, 1.f});

    for (int i = 0; i < 4; i++) {
        data.quad_vertex_buffer_ptr->position =
            transform * data.quad_vertex_position[i];
        data.quad_vertex_buffer_ptr->color = color;
        data.quad_vertex_buffer_ptr->tex_coord = tex_coords[i];
        data.quad_vertex_buffer_ptr->tex_index = tex_index;
        data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
        data.quad_vertex_buffer_ptr++;
    }

    data.quad_index_count += 6;

    data.stats.num_quads++;
}

void Renderer2D::draw_quad(const glm::vec2 &position,
                           const glm::vec2 &size,
                           const Ref<Texture2D> &texture,
                           float tiling_factor,
                           const glm::vec4 &tint_color) {
    draw_quad(
        glm::vec3{position, 0.f}, size, texture, tiling_factor, tint_color);
}

void Renderer2D::draw_quad(const glm::vec3 &position,
                           const glm::vec2 &size,
                           const Ref<Texture2D> &texture,
                           float tiling_factor,
                           const glm::vec4 &tint_color) {
    IVX_PROFILE_FUNCTION();

    if (data.quad_index_count >= Renderer2DData::max_num_indices) {
        flush();
    }

    int tex_index = 0;
    auto view = data.tex_slots | vws::take(data.tex_slot_index);
    if (auto it =
            rg::find(view, texture.get(), [](auto &&e) { return e.get(); });
        it != rg::end(view)) {
        tex_index = static_cast<int>(std::distance(data.tex_slots.begin(), it));
    } else {
        tex_index = data.tex_slot_index;
        data.tex_slots[data.tex_slot_index] = texture;
        data.tex_slot_index++;
    }

    constexpr glm::vec2 tex_coords[]{
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};

    glm::mat4 transform = glm::translate(glm::mat4{1.f}, position) *
                          glm::scale(glm::mat4{1.f}, {size.x, size.y, 1.f});

    for (int i = 0; i < 4; i++) {
        data.quad_vertex_buffer_ptr->position =
            transform * data.quad_vertex_position[i];
        data.quad_vertex_buffer_ptr->color = tint_color;
        data.quad_vertex_buffer_ptr->tex_coord = tex_coords[i];
        data.quad_vertex_buffer_ptr->tex_index = tex_index;
        data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
        data.quad_vertex_buffer_ptr++;
    }

    data.quad_index_count += 6;

    data.stats.num_quads++;
}

void Renderer2D::draw_rotated_quad(const glm::vec2 &position,
                                   const glm::vec2 &size,
                                   float degrees,
                                   const glm::vec4 &color) {
    draw_rotated_quad(glm::vec3{position, 0.f}, size, degrees, color);
}

void Renderer2D::draw_rotated_quad(const glm::vec3 &position,
                                   const glm::vec2 &size,
                                   float degrees,
                                   const glm::vec4 &color) {
    IVX_PROFILE_FUNCTION();

    if (data.quad_index_count >= Renderer2DData::max_num_indices) {
        flush();
    }

    constexpr int tex_index = 0;
    constexpr float tiling_factor = 1.f;
    constexpr glm::vec2 tex_coords[]{
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};

    glm::mat4 transform = glm::translate(glm::mat4{1.f}, position) *
                          glm::rotate(glm::mat4{1.f},
                                      glm::radians(degrees),
                                      glm::vec3{0.f, 0.f, 1.f}) *
                          glm::scale(glm::mat4{1.f}, {size.x, size.y, 1.f});

    for (int i = 0; i < 4; i++) {
        data.quad_vertex_buffer_ptr->position =
            transform * data.quad_vertex_position[i];
        data.quad_vertex_buffer_ptr->color = color;
        data.quad_vertex_buffer_ptr->tex_coord = tex_coords[i];
        data.quad_vertex_buffer_ptr->tex_index = tex_index;
        data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
        data.quad_vertex_buffer_ptr++;
    }

    data.quad_index_count += 6;

    data.stats.num_quads++;
}

void Renderer2D::draw_rotated_quad(const glm::vec2 &position,
                                   const glm::vec2 &size,
                                   float degrees,
                                   const Ref<Texture2D> &texture,
                                   float tiling_factor,
                                   const glm::vec4 &tint_color) {
    draw_rotated_quad(glm::vec3{position, 0.f},
                      size,
                      degrees,
                      texture,
                      tiling_factor,
                      tint_color);
}

void Renderer2D::draw_rotated_quad(const glm::vec3 &position,
                                   const glm::vec2 &size,
                                   float degrees,
                                   const Ref<Texture2D> &texture,
                                   float tiling_factor,
                                   const glm::vec4 &tint_color) {
    IVX_PROFILE_FUNCTION();

    if (data.quad_index_count >= Renderer2DData::max_num_indices) {
        flush();
    }

    int tex_index = 0;
    auto view = data.tex_slots | vws::take(data.tex_slot_index);
    if (auto it =
            rg::find(view, texture.get(), [](auto &&e) { return e.get(); });
        it != rg::end(view)) {
        tex_index = static_cast<int>(std::distance(data.tex_slots.begin(), it));
    } else {
        tex_index = data.tex_slot_index;
        data.tex_slots[data.tex_slot_index] = texture;
        data.tex_slot_index++;
    }

    constexpr glm::vec2 tex_coords[]{
        {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};

    glm::mat4 transform = glm::translate(glm::mat4{1.f}, position) *
                          glm::rotate(glm::mat4{1.f},
                                      glm::radians(degrees),
                                      glm::vec3{0.f, 0.f, 1.f}) *
                          glm::scale(glm::mat4{1.f}, {size.x, size.y, 1.f});

    for (int i = 0; i < 4; i++) {
        data.quad_vertex_buffer_ptr->position =
            transform * data.quad_vertex_position[i];
        data.quad_vertex_buffer_ptr->color = tint_color;
        data.quad_vertex_buffer_ptr->tex_coord = tex_coords[i];
        data.quad_vertex_buffer_ptr->tex_index = tex_index;
        data.quad_vertex_buffer_ptr->tiling_factor = tiling_factor;
        data.quad_vertex_buffer_ptr++;
    }

    data.quad_index_count += 6;

    data.stats.num_quads++;
}

void Renderer2D::reset_stats() {
    std::memset(&data.stats, 0, sizeof(Statistics));
}

Renderer2D::Statistics Renderer2D::get_stats() {
    return data.stats;
}

} // namespace ivx