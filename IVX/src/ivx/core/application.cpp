#include "ivxpch.h"
#include "application.h"
#include "ivx/core/application.h"
#include "ivx/core/base.h"
#include "ivx/core/frame_timer.h"
#include "ivx/core/ref.hpp"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/application_event.h"
#include "ivx/imgui/imgui_layer.h"
#include "ivx/renderer/render_command.h"
#include "ivx/renderer/renderer.h"
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
Application::~Application() {
    IVX_PROFILE_FUNCTION();

    Renderer::shutdown();
}

Application::Application() {
    IVX_PROFILE_FUNCTION();

    IVX_CORE_ASSERT(!instance_, "Application already exists!");
    instance_ = this;
    window_ = Window::create();
    window_->set_event_callback(IVX_BIND_EVENT_FN(Event, on_event));

    Renderer::init();

    imgui_layer_ = make_ref<ImGuiLayer>();
    push_overlay(imgui_layer_.get());
}

void Application::run() {
    IVX_PROFILE_FUNCTION();

    while (running_) {
        IVX_PROFILE_SCOPE("Run Loop");

        // update
        if (!minimized_) {
            float delta_time = FrameTimer::tick();
            {
                IVX_PROFILE_SCOPE("LayerStack::on_update");

                for (auto &&layer : layer_stack_) {
                    layer->on_update(delta_time);
                }
            }
        }

        // render
        imgui_layer_->begin();
        {
            IVX_PROFILE_SCOPE("LayerStack::on_render");

            for (auto &&layer : layer_stack_) {
                layer->on_render();
            }
        }
        imgui_layer_->end();

        // roll and swap buffer
        window_->on_update();
    }
}

void Application::push_layer(Layer *layer) {
    IVX_PROFILE_FUNCTION();

    layer_stack_.push_layer(layer);
    layer->on_attach();
}

void Application::push_overlay(Layer *layer) {
    IVX_PROFILE_FUNCTION();

    layer_stack_.push_overlay(layer);
    layer->on_attach();
}

void Application::on_event(Event &e) {
    IVX_PROFILE_FUNCTION();

    EventDispatcher dispatcher(e);
    dispatcher.dispatch([this](WindowCloseEvent &e) {
        running_ = false;
        return true;
    });

    dispatcher.dispatch([this](WindowResizeEvent &e) {
        IVX_PROFILE_FUNCTION();

        if (e.get_width() == 0 || e.get_height() == 0) {
            minimized_ = true;
        }

        minimized_ = false;
        Renderer::on_window_resize(e);

        return false;
    });

    // IVX_CORE_VERBOSE("{}", e);

    for (auto &&layer : layer_stack_ | vws::reverse) {
        if (e.handled) {
            break;
        }
        layer->on_event(e);
    }
}
} // namespace ivx
