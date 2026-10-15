#pragma once

namespace ivx {
class GraphicsContext {
  public:
    virtual ~GraphicsContext() = default;
    virtual void init() = 0;
    virtual void swap_buffer() = 0;
    // virtual void clear() = 0;
};
} // namespace ivx
