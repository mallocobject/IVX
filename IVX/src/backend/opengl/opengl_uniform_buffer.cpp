#include "ivxpch.h"
#include "opengl_uniform_buffer.h"
#include <glad/glad.h>

namespace ivx {
OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding) {
    glCreateBuffers(1, &ubo_);
    glNamedBufferData(ubo_, size, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLUniformBuffer::~OpenGLUniformBuffer() {
    glDeleteBuffers(1, &ubo_);
}

void OpenGLUniformBuffer::set_data(const void *data,
                                   uint32_t size,
                                   uint32_t offset) {
    glNamedBufferSubData(ubo_, offset, size, data);
}

void OpenGLUniformBuffer::bind(uint32_t binding) {
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_);
}
} // namespace ivx
