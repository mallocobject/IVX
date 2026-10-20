#pragma once

#include "ivx/renderer/buffer.h"
#include <cstdint>

namespace ivx {
class OpenGLVertexBuffer : public VertexBuffer {
  public:
    OpenGLVertexBuffer(uint32_t size);
    OpenGLVertexBuffer(float *vertices, uint32_t count);
    virtual ~OpenGLVertexBuffer();

    virtual void set_data(const void *data, uint32_t size) override;

    virtual uint32_t get_renderer_id() const override {
        return vbo_;
    }

  private:
    uint32_t vbo_{0};
};

class OpenGLIndexBuffer : public IndexBuffer {
  public:
    OpenGLIndexBuffer(uint32_t *indices, uint32_t count);
    virtual ~OpenGLIndexBuffer();

    virtual uint32_t get_count() const override {
        return count_;
    }

    virtual uint32_t get_renderer_id() const override {
        return ibo_;
    }

  private:
    uint32_t ibo_{0};
    uint32_t count_;
};
} // namespace ivx
