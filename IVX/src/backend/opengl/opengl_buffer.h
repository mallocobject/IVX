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

    virtual uint32_t get_count() const override {
        return count_;
    }

  private:
    uint32_t ibo_{0};
    uint32_t count_;
};
} // namespace ivx