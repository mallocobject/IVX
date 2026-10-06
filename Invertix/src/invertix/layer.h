#pragma once

#include "invertix/core.h"
#include "invertix/events/event.h"

namespace invertix {
	class IVX_API Layer
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