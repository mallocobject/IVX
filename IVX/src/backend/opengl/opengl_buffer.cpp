#include "ivxpch.h"
#include "opengl_buffer.h"
#include <cstdint>

#include <glad/glad.h>

namespace ivx {
OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) {
    IVX_PROFILE_FUNCTION();

    glCreateBuffers(1, &vbo_);
    glNamedBufferData(vbo_, size, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32_t count) {
    IVX_PROFILE_FUNCTION();

    glCreateBuffers(1, &vbo_);
    glNamedBufferData(vbo_, count * sizeof(float), vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
    IVX_PROFILE_FUNCTION();

    glDeleteBuffers(1, &vbo_);
}

void OpenGLVertexBuffer::set_data(const void *data, uint32_t size) {
    IVX_PROFILE_FUNCTION();

    glNamedBufferSubData(vbo_, 0, size, data);
}

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indices, uint32_t count)
    : count_(count) {
    IVX_PROFILE_FUNCTION();

    glCreateBuffers(1, &ibo_);
    glNamedBufferData(ibo_, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
    IVX_PROFILE_FUNCTION();

    glDeleteBuffers(1, &ibo_);
}
}; // namespace ivx
