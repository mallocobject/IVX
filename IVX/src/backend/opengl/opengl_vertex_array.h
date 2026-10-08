#pragma once

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
    add_vertex_buffer(std::shared_ptr<VertexBuffer> vertex_buf) override;

    virtual void
    set_index_buffer(std::shared_ptr<IndexBuffer> index_buf) override;

    virtual const std::vector<std::shared_ptr<VertexBuffer>> &
    get_vertex_buffers() const override {
        return vertex_bufs_;
    }

    virtual const std::shared_ptr<IndexBuffer> &
    get_index_buffer() const override {
        return index_buf_;
    }

  private:
    uint32_t vao_{0};
    std::vector<std::shared_ptr<VertexBuffer>> vertex_bufs_;
    std::shared_ptr<IndexBuffer> index_buf_;
};
} // namespace ivx