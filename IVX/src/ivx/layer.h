#pragma once

#include "ivx/core.h"
#include "ivx/events/event.h"

namespace ivx {
	class Layer
	{
	public:
		Layer(std::string name = "Layer");
		virtual ~Layer();

		virtual void on_attach() {}
		virtual void on_detach() {}
		virtual void on_update() {}
		virtual void on_event(Event& event) {}

		// for imgui
		virtual void on_render() {};

		inline std::string_view get_name() const { return debug_name_; }

	protected:
		std::string debug_name_;
	};

}