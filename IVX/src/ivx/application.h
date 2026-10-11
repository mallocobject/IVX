#pragma once

#include "core/ref.hpp"
#include "ivx/core.h"
#include "ivx/imgui/imgui_layer.h"
#include "ivx/layer_stack.h"
#include "ivx/window.h"
#include <memory>

namespace ivx {

class Application {
  public:
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

  protected:
    Application();

  private:
    void on_event(Event &e);

    Ref<Window> window_;
    bool running_{true};

    LayerStack layer_stack_;
    Ref<ImGuiLayer> imgui_layer_;

    inline static Application *instance_{nullptr};
};

// defined by client
Application *create_application();
} // namespace ivx
