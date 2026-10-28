#include "ivxpch.h"
#include "opengl_vertex_array.h"
#include "ivx/core/base.h"

#include "ivx/core/ref.hpp"
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
                              vertex_buf->get_id(),
                              0,
                              static_cast<GLsizei>(layout->get_stride()));

    const auto &elements = layout->get_elements();
    GLuint location = 0;
    bool has_instance_attr = false;

    for (const auto &element : elements) {
        using enum ShaderDataType;
        switch (element.type) {
        case boolean:
        case ivec:
        case ivec2:
        case ivec3:
        case ivec4: {
            glEnableVertexArrayAttrib(vao_, location);
            glVertexArrayAttribIFormat(vao_,
                                       location,
                                       count_of(element.type),
                                       GL_INT,
                                       static_cast<GLuint>(element.offset));
            glVertexArrayAttribBinding(vao_, location, binding_index);
            location++;
            break;
        }
        case vec:
        case vec2:
        case vec3:
        case vec4: {
            glEnableVertexArrayAttrib(vao_, location);
            glVertexArrayAttribFormat(vao_,
                                      location,
                                      count_of(element.type),
                                      GL_FLOAT,
                                      element.normalized ? GL_TRUE : GL_FALSE,
                                      static_cast<GLuint>(element.offset));
            glVertexArrayAttribBinding(vao_, location, binding_index);
            location++;
            break;
        }
        case mat3:
        case mat4: {
            const auto column_count = count_of(element.type); // 3 or 4
            const auto column_bytes = sizeof(float) * column_count;
            for (uint32_t col = 0; col < column_count; ++col) {
                const auto column_offset =
                    static_cast<GLuint>(element.offset + column_bytes * col);

                glEnableVertexArrayAttrib(vao_, location);
                glVertexArrayAttribFormat(vao_,
                                          location,
                                          column_count, // vec3 / vec4
                                          GL_FLOAT,
                                          element.normalized ? GL_TRUE
                                                             : GL_FALSE,
                                          column_offset);
                glVertexArrayAttribBinding(vao_, location, binding_index);
                ++location;
            }
            has_instance_attr = true;
            break;
        }
        default:
            IVX_CORE_ASSERT(false, "Unknown ShaderDataType!");
            break;
        }
    }

    glVertexArrayBindingDivisor(vao_, binding_index, has_instance_attr ? 1 : 0);

    vertex_bufs_.push_back(vertex_buf);
}

void OpenGLVertexArray::set_index_buffer(const Ref<IndexBuffer> &index_buf) {
    IVX_PROFILE_FUNCTION();

    glVertexArrayElementBuffer(vao_, index_buf->get_id());
    index_buf_ = index_buf;
}
} // namespace ivx
