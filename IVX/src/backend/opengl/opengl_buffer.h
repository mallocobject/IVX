#pragma once

#include "ivx/render/buffer.h"
#include <cstdint>

namespace ivx {
class OpenGLVertexBuffer : public VertexBuffer {
  public:
    OpenGLVertexBuffer(float *vertices, uint32_t count);
    virtual ~OpenGLVertexBuffer();

    virtual void bind() override;
    virtual void unbind() override;

  private:
    uint32_t vbo_{0};
};

class OpenGLIndexBuffer : public IndexBuffer {
  public:
    OpenGLIndexBuffer(uint32_t *indices, uint32_t count);
    virtual ~OpenGLIndexBuffer();

    virtual void bind() override;
    virtual void unbind() override;

  private:
    uint32_t ibo_{0};
};
} // namespace ivx