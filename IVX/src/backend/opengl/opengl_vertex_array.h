#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/buffer.h"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace ivx {
class OpenGLVertexArray : public VertexArray {
  public:
    OpenGLVertexArray();
    ~OpenGLVertexArray();
    virtual void bind() override;
    virtual void unbind() override;

    virtual void
    add_vertex_buffer(const Ref<VertexBuffer> &vertex_buf) override;

    virtual void set_index_buffer(const Ref<IndexBuffer> &index_buf) override;

    virtual const std::vector<Ref<VertexBuffer>> &
    get_vertex_buffers() const override {
        return vertex_bufs_;
    }

    virtual const Ref<IndexBuffer> &get_index_buffer() const override {
        return index_buf_;
    }

  private:
    uint32_t vao_{0};
    std::vector<Ref<VertexBuffer>> vertex_bufs_;
    Ref<IndexBuffer> index_buf_;
};
} // namespace ivx
