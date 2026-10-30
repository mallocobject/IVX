#include "ivxpch.h"
#include "application.h"
#include "ivx/core/application.h"
#include "ivx/core/base.h"
#include "ivx/core/frame_timer.h"
#include "ivx/core/ref.hpp"
#include "ivx/core/window.h"
#include "ivx/debug/instrumentor.h"
#include "ivx/event/application_event.h"
#include "ivx/imgui/imgui_layer.h"
#include "ivx/render/render_command.h"
#include "ivx/render/render_pass.h"
#include "ivx/render/renderer.h"
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
Application::~Application() {
    IVX_PROFILE_FUNCTION();

    // Tear the layers down while the window and the GL context still exist:
    // their destructors release GPU objects, and the editor also owns script
    // instances whose code lives in a DLL that must not be unloaded before they
    // are destroyed.
    layer_stack_.clear();

    Renderer::shutdown();
}

Application::Application(const std::string &title,
                         uint32_t width,
                         uint32_t height) {
    IVX_PROFILE_FUNCTION();

    IVX_CORE_ASSERT(!instance_, "Application already exists!");
    instance_ = this;
    window_ = Window::create(WindowProps{title, width, height});
    window_->set_event_callback(IVX_BIND_FN(on_event));

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
            float dt = FrameTimer::tick();
            {
                IVX_PROFILE_SCOPE("LayerStack::on_update");

                for (auto &&layer : layer_stack_) {
                    layer->on_update(dt);
                }
            }
        }

        // The default framebuffer pass. Layers that render do it in on_update()
        // into their own framebuffer, so this pass owns the screen and nothing
        // else; the UI is drawn into it by ImGuiLayer::end() below. Clearing
        // here instead of through RenderCommand keeps a single owner for
        // "clear" - a pass - whether the target is a framebuffer or the screen.
        RenderPassSpecification screen_spec;
        screen_spec.clear_color = glm::vec4{0.1f, 0.1f, 0.1f, 1.f};
        // The values RendererAPI::init() starts with, so the first pass that
        // snapshots the state sees the engine defaults.
        screen_spec.depth_test = true;
        screen_spec.depth_write = true;
        screen_spec.blend = true;
        screen_spec.cull_faces = true;
        RenderPass screen_pass{screen_spec};
        screen_pass.begin();

        // UI phase: the layers only build ImGui commands here, the backend
        // submits them from ImGuiLayer::end() below. Rendering happens in
        // on_update().
        imgui_layer_->begin();
        {
            IVX_PROFILE_SCOPE("LayerStack::on_imgui");

            for (auto &&layer : layer_stack_) {
                layer->on_imgui();
            }
        }
        imgui_layer_->end();

        screen_pass.end();

        // roll and swap buffer
        window_->on_update();
    }
}

void Application::close() {
    running_ = false;
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

        minimized_ = (e.get_width() == 0 || e.get_height() == 0);
        if (!minimized_) {
            Renderer::on_window_resize(e);
        }

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
