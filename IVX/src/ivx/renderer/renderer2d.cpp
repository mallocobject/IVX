#include "ivxpch.h"
#include "renderer2d.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "ivx/renderer/renderer.h"
#include "ivx/renderer/shader.h"
#include "ivx/renderer/texture.h"
#include "ivx/renderer/vertex_array.h"
#include <cstdint>

namespace ivx {
struct Renderer2DStorage {
    Ref<VertexArray> quad_vertex_array;
    Ref<Texture2D> white_texture;
    Ref<Shader> texture_shader;

    glm::mat4 vp;
};

static Renderer2DStorage *data;

void Renderer2D::init() {
    data = new Renderer2DStorage;
    data->quad_vertex_array = VertexArray::create();

    float vertices[5 * 4] = {
        -300.f, -300.f, 0.f, 0.f, 0.f, 300.f,  -300.f, 0.f, 1.f, 0.f,
        300.f,  300.f,  0.f, 1.f, 1.f, -300.f, 300.f,  0.f, 0.f, 1.f,
    };

    auto vertex_buf =
        ivx::VertexBuffer::create(vertices, sizeof(vertices) / sizeof(float));

    ivx::Ref<ivx::BufferLayout> bl{new ivx::BufferLayout{
        {"a_Position", ivx::ShaderDataType::vec3, false},
        {"a_Texcoord", ivx::ShaderDataType::vec2, false},
    }};
    vertex_buf->set_buffer_layout(bl);

    data->quad_vertex_array->add_vertex_buffer(vertex_buf);

    uint32_t indices[] = {
        0,
        1,
        2,
        0,
        2,
        3,
    };

    auto index_buf =
        ivx::IndexBuffer::create(indices, sizeof(indices) / sizeof(uint32_t));

    data->quad_vertex_array->set_index_buffer(index_buf);

    data->white_texture = Texture2D::create(1, 1);
    uint32_t white_texture_data = -1; // 0xffffffff
    data->white_texture->set_data(&white_texture_data, sizeof(uint32_t));

    data->texture_shader = Shader::create(SHADERS "texture.glsl");
    data->texture_shader->set("u_Texture", 0);
}

void Renderer2D::shutdown() {
    delete data;
}

void Renderer2D::begin_scene(const Ref<OrthoGraphicCamera> &camera) {
    data->vp = camera->get_proj_matrix() * camera->get_view_matrix();
}

void Renderer2D::end_scene() {
}

void Renderer2D::draw_quad(const glm::vec2 &position,
                           const glm::vec2 &size,
                           const glm::vec4 &color) {
    draw_quad(glm::vec3{position, 0.f}, size, color);
}

void Renderer2D::draw_quad(const glm::vec3 &position,
                           const glm::vec2 &size,
                           const glm::vec4 &color) {
    data->texture_shader->set("u_Color", color);
    glm::mat4 transform = glm::translate(glm::mat4{1.f}, position) *
                          glm::scale(glm::mat4{1.f}, {size.x, size.y, 1.f});
    data->texture_shader->set("mvp", data->vp * transform);
    data->white_texture->bind();
    data->texture_shader->bind();
    data->quad_vertex_array->bind();
    RenderCommand::draw(data->quad_vertex_array);
    data->texture_shader->unbind();
    data->quad_vertex_array->unbind();
}

void Renderer2D::draw_quad(const glm::vec2 &position,
                           const glm::vec2 &size,
                           const Ref<Texture2D> &texture) {
    draw_quad(glm::vec3{position, 0.f}, size, texture);
}

void Renderer2D::draw_quad(const glm::vec3 &position,
                           const glm::vec2 &size,
                           const Ref<Texture2D> &texture) {
    glm::mat4 transform = glm::translate(glm::mat4{1.f}, position) *
                          glm::scale(glm::mat4{1.f}, {size.x, size.y, 1.f});
    texture->bind();
    data->texture_shader->set("mvp", data->vp * transform);
    data->texture_shader->set("u_Color", glm::vec4(1.f));
    data->texture_shader->bind();
    data->quad_vertex_array->bind();
    RenderCommand::draw(data->quad_vertex_array);
    data->texture_shader->unbind();
    data->quad_vertex_array->unbind();
}
} // namespace ivx