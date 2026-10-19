#pragma once

#include "ivx/core/ref_counted.hpp"

namespace ivx {
class GraphicsContext : public RefCounted {
  public:
    virtual ~GraphicsContext() = default;
    virtual void init() = 0;
    virtual void swap_buffer() = 0;

    static Ref<GraphicsContext> create(void *window_handle);
};
} // namespace ivx
