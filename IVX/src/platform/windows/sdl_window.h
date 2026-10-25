#pragma once

#include "ivx/core/window.h"
#include "ivx/renderer/graphics_context.h"

#include <SDL3/SDL.h>

namespace ivx {
class SDLWindow : public Window {
  public:
    SDLWindow(const WindowProps &props);
    virtual ~SDLWindow();

    void on_update() override;
    uint32_t get_width() const override {
        return data_.width;
    }
    uint32_t get_height() const override {
        return data_.height;
    }
    void set_event_callback(const EventCallbackFn &callback) override {
        data_.event_callback = callback;
    }
    void set_v_sync(bool enabled) override;
    bool is_v_sync() const override {
        return data_.v_sync;
    }
    void set_text_input_enabled(bool enabled) override;
    void *get_native_window() override {
        return window_handle_;
    }

    void *get_native_gl_context() override {
        return context_ ? context_->get_native_context() : nullptr;
    }

    void set_native_event_callback(NativeEventCallbackFn cb) override {
        native_cb_ = std::move(cb);
    }

  private:
    void init(const WindowProps &props);
    void shutdown();
    void process_event(const SDL_Event &e);

    SDL_Window *window_handle_{nullptr};
    Ref<GraphicsContext> context_;
    NativeEventCallbackFn native_cb_;
    struct WindowData {
        std::string title;
        uint32_t width{0}, height{0};
        bool v_sync{true};
        EventCallbackFn event_callback;
    } data_;
};
} // namespace ivx