#include "ivxpch.h"
#include "backend/opengl/opengl_buffer.h"
#include <cstdint>

#include <glad/glad.h>

namespace ivx {
OpenGLVertexBuffer::OpenGLVertexBuffer(float *vertices, uint32_t count) {
    glCreateBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(
        GL_ARRAY_BUFFER, count * sizeof(float), vertices, GL_STATIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
    glDeleteBuffers(1, &vbo_);
}

void OpenGLVertexBuffer::bind() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
}

void OpenGLVertexBuffer::unbind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t *indices, uint32_t count)
    : count_(count) {
    glCreateBuffers(1, &ibo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 count * sizeof(uint32_t),
                 indices,
                 GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
    glDeleteBuffers(1, &ibo_);
}

void OpenGLIndexBuffer::bind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
}

void OpenGLIndexBuffer::unbind() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
}; // namespace ivx
