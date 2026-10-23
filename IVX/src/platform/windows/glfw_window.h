#pragma once

#include "ivx/core/window.h"
#include "ivx/renderer/graphics_context.h"

#include <GLFW/glfw3.h>

namespace ivx {

class GLFWWindow : public Window {
  public:
    GLFWWindow(const WindowProps &props);
    virtual ~GLFWWindow();

    void on_update() override;

    inline uint32_t get_width() const override {
        return data_.width;
    }
    inline uint32_t get_height() const override {
        return data_.height;
    }

    // Window attributes
    inline void set_event_callback(const EventCallbackFn &callback) override {
        data_.event_callback = callback;
    }
    void set_v_sync(bool enabled) override;

    bool is_v_sync() const override {
        return data_.v_sync;
    }

    void *get_native_window() override {
        return reinterpret_cast<void *>(window_handle_);
    }

  private:
    virtual void init(const WindowProps &props);
    virtual void shutdown();

    GLFWwindow *window_handle_{nullptr};
    Ref<GraphicsContext> context_;

    struct WindowData {
        std::string title;
        uint32_t width{0};
        uint32_t height{0};
        bool v_sync{true};

        EventCallbackFn event_callback;
    } data_;
};

} // namespace ivx
