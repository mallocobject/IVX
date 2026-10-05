#pragma once

#include "invertix/core.h"
#include "invertix/window.h"
#include "invertix/layer_stack.h"
#include "invertix/events/event.h"
#include "invertix/events/application_event.h"

namespace invertix {
	class IVX_API Application
	{
	public:
		Application();
		virtual ~Application();

		static Application& get() {
			return *instance_;
		}

		Window& get_window() {
			return *window_;
		}

		void run();

		void push_layer(Layer* layer);
		void push_overlay(Layer* layer);

	private:
		void on_event(Event& e);
		bool on_window_close(WindowCloseEvent& e);

		std::unique_ptr<Window> window_;
		bool running_{ true };

		LayerStack layer_stack_;

		inline static Application* instance_{ nullptr };
	};


	// defined by client
	Application* create_application();
}

