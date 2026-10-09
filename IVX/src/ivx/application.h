#pragma once

#include "ivx/core.h"
#include "ivx/event/application_event.h"
#include "ivx/event/event.h"
#include "ivx/layer_stack.h"
#include "ivx/window.h"
#include "render/buffer.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "render/vertex_array.h"
#include <memory>

// #include "ivx/imgui/imgui_layer.h"

namespace ivx {
class ImGuiLayer;

class Application {
  public:
    Application();
    virtual ~Application() = default;

    static Application &get() {
        return *instance_;
    }

    Window &get_window() {
        return *window_;
    }

    void run();

    void push_layer(Layer *layer);
    void push_overlay(Layer *layer);

  private:
    void on_event(Event &e);
    bool on_window_close(WindowCloseEvent &e);

    std::unique_ptr<Window> window_;
    bool running_{true};

    LayerStack layer_stack_;
    ImGuiLayer *imgui_layer_{nullptr};
    std::shared_ptr<Shader> shader_;
    std::shared_ptr<Shader> shader2_;

    inline static Application *instance_{nullptr};

    std::shared_ptr<VertexArray> vertex_array_;
    std::shared_ptr<VertexArray> vertex_array2_;
};

// defined by client
Application *create_application();
} // namespace ivx
