#pragma once

#include "ivx/render/buffer.h"
#include <memory>
#include <utility>
#include <vector>
namespace ivx {
class VertexArray {
  public:
    virtual ~VertexArray() = default;
    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void
    add_vertex_buffer(std::shared_ptr<VertexBuffer> vertex_buf) = 0;

    virtual void set_index_buffer(std::shared_ptr<IndexBuffer> index_buf) = 0;

    virtual const std::vector<std::shared_ptr<VertexBuffer>> &
    get_vertex_buffers() const = 0;

    virtual const std::shared_ptr<IndexBuffer> &get_index_buffer() const = 0;

    static VertexArray *create();
};
} // namespace ivx