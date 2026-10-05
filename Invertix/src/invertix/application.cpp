#include "ivx_pch.h"
#include "application.h"

#include <GLFW/glfw3.h>

namespace invertix {

#define BIND_EVENT_FN(x) [this](auto& e){ return x(e); } 

	Application::Application()
	{
		window_ = std::unique_ptr<Window>(Window::create());
		window_->set_event_callback(BIND_EVENT_FN(on_event));
	}

	Application::~Application()
	{
	}

	void Application::run()
	{
		while (running_) {
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);

			// do something
			for (auto&& layer : layer_stack_) {
				layer->on_update();
			}


			window_->on_update();
		}
	}

	void Application::push_layer(Layer* layer)
	{
		layer_stack_.push_layer(layer);
	}

	void Application::push_overlay(Layer* layer)
	{
		layer_stack_.push_overlay(layer);
	}

	void Application::on_event(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(on_window_close));

		IVX_CORE_TRACE("{}", e);

		for (auto&& layer : layer_stack_ | vws::reverse) {
			if (e.handled) {
				break;
			}
			layer->on_event(e);
		}
	}

	bool Application::on_window_close(WindowCloseEvent& e)
	{
		running_ = false;
		return true;
	}
}