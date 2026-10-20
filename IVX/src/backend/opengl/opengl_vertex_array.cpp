#include "ivxpch.h"
#include "opengl_vertex_array.h"
#include "ivx/core/base.h"

#include "ivx/core/ref.hpp"
#include "ivx/renderer/buffer.h"
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
    case ivec:
        return GL_INT;
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
    IVX_PROFILE_FUNCTION();

    glCreateVertexArrays(1, &vao_);
}

OpenGLVertexArray::~OpenGLVertexArray() {
    IVX_PROFILE_FUNCTION();

    glDeleteVertexArrays(1, &vao_);
}

void OpenGLVertexArray::bind() {
    IVX_PROFILE_FUNCTION();

    glBindVertexArray(vao_);
}

void OpenGLVertexArray::unbind() {
    IVX_PROFILE_FUNCTION();

    glBindVertexArray(0);
}

void OpenGLVertexArray::add_vertex_buffer(const Ref<VertexBuffer> &vertex_buf) {
    IVX_PROFILE_FUNCTION();

    const auto &layout = vertex_buf->get_buffer_layout();
    IVX_CORE_ASSERT(layout && layout->get_elements().size(),
                    "Vertex Buffer has no layout!");

    const auto binding_index = static_cast<uint32_t>(vertex_bufs_.size());
    glVertexArrayVertexBuffer(vao_,
                              binding_index,
                              vertex_buf->get_renderer_id(),
                              0,
                              static_cast<GLsizei>(layout->get_stride()));

    const auto &elements = layout->get_elements();
    for (int i = 0; i < elements.size(); i++) {
        const auto &element = elements[i];
        const auto location = static_cast<uint32_t>(i);

        glEnableVertexArrayAttrib(vao_, location);

        auto type = shader_data_type_to_opengl_base_type(element.type);
        if (type == GL_FLOAT) {
            glVertexArrayAttribFormat(vao_,
                                      location,
                                      count_of(element.type),
                                      GL_FLOAT,
                                      element.normalized ? GL_TRUE : GL_FALSE,
                                      static_cast<GLuint>(element.offset));
        } else if (type == GL_INT) {
            glVertexArrayAttribIFormat(vao_,
                                       location,
                                       count_of(element.type),
                                       GL_INT,
                                       static_cast<GLuint>(element.offset));
        }

        glVertexArrayAttribBinding(vao_, location, binding_index);
    }

    vertex_bufs_.push_back(vertex_buf);
}

void OpenGLVertexArray::set_index_buffer(const Ref<IndexBuffer> &index_buf) {
    IVX_PROFILE_FUNCTION();

    glVertexArrayElementBuffer(vao_, index_buf->get_renderer_id());
    index_buf_ = index_buf;
}
} // namespace ivx
