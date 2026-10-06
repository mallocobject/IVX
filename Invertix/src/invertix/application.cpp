#include "ivx_pch.h"
#include "application.h"
#include "invertix/core.h"
#include "invertix/input.h"
#include "invertix/imgui/imgui_layer.h"

#include <glm/glm.hpp>

namespace invertix {
	Application::Application()
	{
		IVX_CORE_ASSERT(!instance_, "Application already exists!");
		instance_ = this;
		window_ = std::unique_ptr<Window>(Window::create());
		window_->set_event_callback(IVX_BIND_EVENT_FN(Event, on_event));

		imgui_layer_ = new ImGuiLayer;
		push_overlay(imgui_layer_);
	}

	Application::~Application()
	{
	}


	void Application::run()
	{
		while (running_) {
			window_->clear();

			// do something
			for (auto&& layer : layer_stack_) {
				layer->on_update();
			}

			imgui_layer_->begin();
			for (auto&& layer : layer_stack_) {
				layer->on_render();
			}
			imgui_layer_->end();

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
		dispatcher.dispatch(IVX_BIND_EVENT_FN(WindowCloseEvent, on_window_close));

		//IVX_CORE_TRACE("{}", e);

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