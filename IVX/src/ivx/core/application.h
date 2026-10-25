#pragma once

#include "ivx/core/base.h"
#include "ivx/core/layer_stack.h"
#include "ivx/core/ref.hpp"
#include "ivx/core/window.h"
#include "ivx/imgui/imgui_layer.h"
#include <memory>

int main(int argc, char *argv[]);

namespace ivx {

class Application {
    friend int ::main(int argc, char *argv[]);

  public:
    virtual ~Application();

    static Application &get() {
        return *instance_;
    }

    Window &get_window() {
        return *window_;
    }

    void close();

    void push_layer(Layer *layer);
    void push_overlay(Layer *layer);

    Ref<ImGuiLayer> imgui_layer() const {
        return imgui_layer_;
    }

  protected:
    // Width/height are the initial window size. The default fits a plain game view;
    // an editor with several panels needs more room.
    Application(const std::string &title = "IVX Engine",
                uint32_t width = 1280,
                uint32_t height = 720);

  private:
    void run();
    void on_event(Event &e);

    Ref<Window> window_;
    bool running_{true};
    bool minimized_{false};

    LayerStack layer_stack_;
    Ref<ImGuiLayer> imgui_layer_;

    inline static Application *instance_{nullptr};
};

// defined by client
Application *create_application();
} // namespace ivx
