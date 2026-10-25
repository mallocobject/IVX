#pragma once

#include "ivx/core/base.h"
#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <functional>
#include <string>

namespace ivx {

class Event;

struct WindowProps {
    std::string title{"IVX Engine"};
    uint32_t width{1280};
    uint32_t height{720};
};

// Interface representing a desktop system based Window
class Window : public RefCounted {
  public:
    using EventCallbackFn = std::function<void(Event &)>;

    virtual ~Window() = default;

    virtual void on_update() = 0;
    // virtual void clear() = 0;

    virtual uint32_t get_width() const = 0;
    virtual uint32_t get_height() const = 0;

    // Window attributes
    virtual void set_event_callback(const EventCallbackFn &callback) = 0;
    virtual void set_v_sync(bool enabled) = 0;
    virtual bool is_v_sync() const = 0;

    virtual void *get_native_window() = 0;

    // Graphics context handle (SDL_GLContext for the SDL3 backend); nullptr
    // when the backend does not expose one.
    virtual void *get_native_gl_context() {
        return nullptr;
    }

    // Raw platform event hook: backends with an event queue (SDL3) forward
    // every native event here so ImGui can consume it, while callback based
    // backends (GLFW) leave it unused.
    using NativeEventCallbackFn = std::function<void(const void *)>;
    virtual void set_native_event_callback(NativeEventCallbackFn) {
    }

    static Ref<Window> create(const WindowProps &props = WindowProps{});
};
} // namespace ivx
