#pragma once

#include "invertix/events/event.h"

using MouseCode = int;

namespace invertix {

	class IVX_API MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: mouse_x_(x), mouse_y_(y) {
		}

		float get_x() const { return mouse_x_; }
		float get_y() const { return mouse_y_; }

		std::string to_string() const override
		{
			return std::format("MouseMovedEvent: {}, {}", mouse_x_, mouse_y_);
		}

		EVENT_CLASS_TYPE(kMouseMoved)
			EVENT_CLASS_CATEGORY(kEventCategoryMouse | kEventCategoryInput)

	private:
		float mouse_x_;
		float mouse_y_;
	};

	class IVX_API MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float x_offset, float y_offset)
			: x_offset_(x_offset), y_offset_(y_offset) {
		}

		float get_x_offset() const { return x_offset_; }
		float get_y_offset() const { return y_offset_; }

		std::string to_string() const override
		{
			return std::format("MouseScrolledEvent: {}, {}", x_offset_, y_offset_);
		}

		EVENT_CLASS_TYPE(kMouseScrolled)
			EVENT_CLASS_CATEGORY(kEventCategoryMouse | kEventCategoryInput)
	private:
		float x_offset_;
		float y_offset_;
	};

	class IVX_API MouseButtonEvent : public Event
	{
	public:
		MouseCode get_mouse_button() const { return button_; }

		EVENT_CLASS_CATEGORY(kEventCategoryMouse | kEventCategoryInput | kEventCategoryMouseButton)
	protected:
		MouseButtonEvent(MouseCode button)
			: button_(button) {
		}

		MouseCode button_;
	};

	class IVX_API MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button)
			: MouseButtonEvent(button) {
		}

		std::string to_string() const override
		{
			return std::format("MouseButtonPressedEvent: {}", button_);
		}

		EVENT_CLASS_TYPE(kMouseButtonPressed)
	};

	class IVX_API MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseCode button)
			: MouseButtonEvent(button) {
		}

		std::string to_string() const override
		{
			return std::format("MouseButtonReleasedEvent: {}", button_);
		}

		EVENT_CLASS_TYPE(kMouseButtonReleased)
	};
}

//IVX_EVENT_FORMATTER(invertix::MouseMovedEvent);
//IVX_EVENT_FORMATTER(invertix::MouseScrolledEvent);
//IVX_EVENT_FORMATTER(invertix::MouseButtonPressedEvent);
//IVX_EVENT_FORMATTER(invertix::MouseButtonReleasedEvent);