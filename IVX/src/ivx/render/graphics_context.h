#pragma once

#include "ivx/core/ref_counted.hpp"

namespace ivx {
class GraphicsContext : public RefCounted {
  public:
    virtual ~GraphicsContext() = default;
    virtual void init() = 0;
    virtual void swap_buffer() = 0;

    // Backend graphics context handle (e.g. SDL_GLContext). Needed by ImGui's
    // SDL3 backend to share resources with its platform windows.
    virtual void *get_native_context() {
        return nullptr;
    }

    static Ref<GraphicsContext> create(void *window_handle);
};
} // namespace ivx
