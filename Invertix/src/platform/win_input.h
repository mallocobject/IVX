#pragma once

#include "invertix/Input.h"

namespace invertix {

	class WinInput : public Input
	{
	protected:
		virtual bool is_key_pressed_impl(int keycode) override;
		virtual bool is_mouse_button_pressed_impl(int button) override;
		virtual std::pair<float, float> get_mouse_pos_impl() override;
	};

}
