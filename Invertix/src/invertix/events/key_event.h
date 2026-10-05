#pragma once

#include "invertix/events/event.h"

using KeyCode = int;

namespace invertix {

	class KeyEvent : public Event
	{
	public:
		inline KeyCode get_key_code() const { return key_code_; }
		EVENT_CLASS_CATEGORY(kEventCategoryKeyboard | kEventCategoryInput)
	protected:
		KeyEvent(KeyCode keycode)
			: key_code_(keycode) {
		}

		KeyCode key_code_;
	};

	class IVX_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keycode, bool is_repeat = false)
			: KeyEvent(keycode), is_repeat_(is_repeat) {
		}

		bool is_repeat() const { return is_repeat_; }

		std::string to_string() const override
		{
			return std::format("KeyPressedEvent: {} (repeat = {})", key_code_, is_repeat_);
		}

		EVENT_CLASS_TYPE(kKeyPressed)
	private:
		bool is_repeat_{ false };
	};

	class IVX_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keycode)
			: KeyEvent(keycode) {
		}

		std::string to_string() const override
		{
			return std::format("KeyReleasedEvent: {}", key_code_);
		}

		EVENT_CLASS_TYPE(kKeyReleased)
	};

	class IVX_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode keycode)
			: KeyEvent(keycode) {
		}

		std::string to_string() const override
		{
			return std::format("KeyTypedEvent: {}", key_code_);
		}

		EVENT_CLASS_TYPE(kKeyTyped)
	};
}

//IVX_EVENT_FORMATTER(invertix::KeyPressedEvent);
//IVX_EVENT_FORMATTER(invertix::KeyReleasedEvent);
//IVX_EVENT_FORMATTER(invertix::KeyTypedEvent);