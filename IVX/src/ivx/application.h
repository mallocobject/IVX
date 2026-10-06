#pragma once

#include "ivx/core.h"
#include "ivx/window.h"
#include "ivx/layer_stack.h"
#include "ivx/events/event.h"
#include "ivx/events/application_event.h"
//#include "ivx/imgui/imgui_layer.h"

namespace ivx {
	class ImGuiLayer;

	class Application
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
		ImGuiLayer* imgui_layer_{ nullptr };

		inline static Application* instance_{ nullptr };
	};


	// defined by client
	Application* create_application();
}

