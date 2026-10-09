#include "ivxpch.h"
#include "opengl_vertex_array.h"
#include "ivx/core.h"

#include "ivx/render/buffer.h"
#include <glad/glad.h>

namespace ivx {
static GLenum shader_data_type_to_opengl_base_type(ShaderDataType type) {
    using enum ShaderDataType;
    switch (type) {
    case boolean:
        return GL_BOOL;
    case vec:
        return GL_FLOAT;
    case vec2:
        return GL_FLOAT;
    case vec3:
        return GL_FLOAT;
    case vec4:
        return GL_FLOAT;
    case ivec2:
        return GL_INT;
    case ivec3:
        return GL_INT;
    case ivec4:
        return GL_INT;
    case mat3:
        return GL_FLOAT;
    case mat4:
        return GL_FLOAT;
    }

    IVX_CORE_ASSERT(false, "Unknown ShaderDataType!");
    return 0;
}

OpenGLVertexArray::OpenGLVertexArray() {
    glCreateVertexArrays(1, &vao_);
}

OpenGLVertexArray::~OpenGLVertexArray() {
    glDeleteVertexArrays(1, &vao_);
}

void OpenGLVertexArray::bind() {
    glBindVertexArray(vao_);
}

void OpenGLVertexArray::unbind() {
    glBindVertexArray(0);
}

void OpenGLVertexArray::add_vertex_buffer(
    std::shared_ptr<VertexBuffer> vertex_buf) {
    IVX_CORE_ASSERT(vertex_buf->get_buffer_layout().get_elements().size(),
                    "Vertex Buffer has no layout!");

    glBindVertexArray(vao_);
    vertex_buf->bind();

    auto &buffer_layout_vec = vertex_buf->get_buffer_layout().get_elements();
    for (int i = 0; i < buffer_layout_vec.size(); i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(
            i,
            count_of(buffer_layout_vec[i].type),
            shader_data_type_to_opengl_base_type(buffer_layout_vec[i].type),
            buffer_layout_vec[i].normalized ? GL_TRUE : GL_FALSE,
            vertex_buf->get_buffer_layout().get_stride(),
            reinterpret_cast<const void *>(
                static_cast<std::uintptr_t>(buffer_layout_vec[i].offset)));
    }

    glBindVertexArray(0);

    vertex_bufs_.push_back(std::move(vertex_buf));
}

void OpenGLVertexArray::set_index_buffer(
    std::shared_ptr<IndexBuffer> index_buf) {
    glBindVertexArray(vao_);
    index_buf->bind();

    glBindVertexArray(0);

    index_buf_ = std::move(index_buf);
}
} // namespace ivx