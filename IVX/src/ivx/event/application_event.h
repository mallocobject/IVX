#pragma once

#include "ivx/event/event.h"

namespace ivx {

class WindowResizeEvent : public Event {
  public:
    WindowResizeEvent(uint32_t width, uint32_t height)
        : width_(width), height_(height) {
    }

    uint32_t get_width() const {
        return width_;
    }
    uint32_t get_height() const {
        return height_;
    }

    std::string to_string() const override {
        return std::format("WindowResizeEvent: {}, {}", width_, height_);
    }

    EVENT_CLASS_TYPE(kWindowResize)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
  private:
    uint32_t width_;
    uint32_t height_;
};

class WindowCloseEvent : public Event {
  public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(kWindowClose)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};

class AppTickEvent : public Event {
  public:
    AppTickEvent() = default;

    EVENT_CLASS_TYPE(kAppTick)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};

// Emitted when the OS window gains or loses focus (SDL_EVENT_WINDOW_FOCUS_*).
// The GLFW back end does not emit it yet.
class WindowFocusEvent : public Event {
  public:
    explicit WindowFocusEvent(bool focused) : focused_(focused) {
    }

    bool is_focused() const {
        return focused_;
    }

    std::string to_string() const override {
        return std::format("WindowFocusEvent: focused={}", focused_);
    }

    EVENT_CLASS_TYPE(kWindowFocus)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
  private:
    bool focused_;
};

class AppUpdateEvent : public Event {
  public:
    AppUpdateEvent() = default;

    EVENT_CLASS_TYPE(kAppUpdate)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};

class AppRenderEvent : public Event {
  public:
    AppRenderEvent() = default;

    EVENT_CLASS_TYPE(kAppRender)
    EVENT_CLASS_CATEGORY(kEventCategoryApplication)
};
} // namespace ivx
