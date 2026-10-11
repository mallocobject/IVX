#include "ivxpch.h"
#include "application.h"
#include "core.h"
#include "core/ref.hpp"
#include "event/application_event.h"

#include "imgui/imgui_layer.h"
#include "render/render_command.h"
#include "render/renderer.h"
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
Application::Application() {
    IVX_CORE_ASSERT(!instance_, "Application already exists!");
    instance_ = this;
    window_ = Window::create();
    window_->set_event_callback(IVX_BIND_EVENT_FN(Event, on_event));

    Renderer::init();

    imgui_layer_ = make_ref<ImGuiLayer>();
    push_overlay(imgui_layer_.get());
}

void Application::run() {
    while (running_) {
        // clear
        RenderCommand::clear();

        // update
        for (auto &&layer : layer_stack_) {
            layer->on_update();
        }

        // render
        imgui_layer_->begin();
        for (auto &&layer : layer_stack_) {
            layer->on_render();
        }
        imgui_layer_->end();

        // roll and swap buffer
        window_->on_update();
    }
}

void Application::push_layer(Layer *layer) {
    layer_stack_.push_layer(layer);
}

void Application::push_overlay(Layer *layer) {
    layer_stack_.push_overlay(layer);
}

void Application::on_event(Event &e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch([this](WindowCloseEvent &e) {
        running_ = false;
        return true;
    });

    // IVX_CORE_TRACE("{}", e);

    for (auto &&layer : layer_stack_ | vws::reverse) {
        if (e.handled) {
            break;
        }
        layer->on_event(e);
    }
}
} // namespace ivx