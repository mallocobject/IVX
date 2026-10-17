#include "ivxpch.h"
#include "application.h"
#include "ivx/core/base.h"
#include "ivx/core/frame_timer.h"
#include "ivx/core/ref.hpp"
#include "ivx/event/application_event.h"
#include "ivx/imgui/imgui_layer.h"
#include "ivx/renderer/render_command.h"
#include "ivx/renderer/renderer.h"
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
        if (!minimized_) {
            float delta_time = FrameTimer::tick();
            for (auto &&layer : layer_stack_) {
                layer->on_update(delta_time);
            }
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

    dispatcher.dispatch([this](WindowResizeEvent &e) {
        if (e.get_width() == 0 || e.get_height() == 0) {
            minimized_ = true;
        }

        minimized_ = false;
        Renderer::on_window_resize(e);

        return false;
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
