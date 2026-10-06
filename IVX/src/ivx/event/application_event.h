#pragma once

#include "ivx/event/event.h"

namespace ivx {

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: width_(width), height_(height) {
		}

		unsigned int get_width() const { return width_; }
		unsigned int get_height() const { return height_; }

		std::string to_string() const override
		{
			return std::format("WindowResizeEvent: {}, {}", width_, height_);
		}

		EVENT_CLASS_TYPE(kWindowResize)
			EVENT_CLASS_CATEGORY(kEventCategoryApplication)
	private:
		unsigned int width_;
		unsigned int height_;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(kWindowClose)
			EVENT_CLASS_CATEGORY(kEventCategoryApplication)
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() = default;

		EVENT_CLASS_TYPE(kAppTick)
			EVENT_CLASS_CATEGORY(kEventCategoryApplication)
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() = default;

		EVENT_CLASS_TYPE(kAppUpdate)
			EVENT_CLASS_CATEGORY(kEventCategoryApplication)
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() = default;

		EVENT_CLASS_TYPE(kAppRender)
			EVENT_CLASS_CATEGORY(kEventCategoryApplication)
	};
}
