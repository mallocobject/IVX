#pragma once

#include "ivx/core.h"
#include <utility>

namespace ivx {

	class Input
	{
	public:
		virtual ~Input() {}

		static bool is_key_pressed(int keycode) { return instance_->is_key_pressed_impl(keycode); }

		static bool IsMouseButtonPressed(int button) { return instance_->is_mouse_button_pressed_impl(button); }
		static std::pair<float, float> get_mouse_pos() { return instance_->get_mouse_pos_impl(); }
	protected:
		virtual bool is_key_pressed_impl(int keycode) = 0;
		virtual bool is_mouse_button_pressed_impl(int button) = 0;
		virtual std::pair<float, float> get_mouse_pos_impl() = 0;
	private:
		static Input* instance_;
	};

}