#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/renderer/buffer.h"
#include <memory>
#include <utility>
#include <vector>

namespace ivx {
class VertexArray : public RefCounted {
  public:
    virtual ~VertexArray() = default;
    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual void add_vertex_buffer(const Ref<VertexBuffer> &vertex_buf) = 0;

    virtual void set_index_buffer(const Ref<IndexBuffer> &index_buf) = 0;

    virtual const std::vector<Ref<VertexBuffer>> &
    get_vertex_buffers() const = 0;

    virtual const Ref<IndexBuffer> &get_index_buffer() const = 0;

    static Ref<VertexArray> create();
};
} // namespace ivx
