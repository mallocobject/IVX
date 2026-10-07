#pragma once

#include "ivxpch.h"
#include "ivx/core.h"

namespace ivx {

class Event;

struct WindowProps {
    std::string title{"IVX Engine"};
    uint32_t width{1280};
    uint32_t height{720};
};

// Interface representing a desktop system based Window
class Window {
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

    static Window *create(const WindowProps &props = WindowProps{});
};
} // namespace ivx