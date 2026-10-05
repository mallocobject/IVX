#include "ivx_pch.h"
#include "win_input.h"
#include "invertix/Application.h"
#include <GLFW/glfw3.h>

namespace invertix {
	Input* Input::instance_ = new WinInput;

	bool WinInput::is_key_pressed_impl(int keycode) {
		auto window = reinterpret_cast<GLFWwindow*>(Application::get().get_window().get_native_window());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WinInput::is_mouse_button_pressed_impl(int button) {
		auto window = reinterpret_cast<GLFWwindow*>(Application::get().get_window().get_native_window());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> WinInput::get_mouse_pos_impl() {
		auto window = reinterpret_cast<GLFWwindow*>(Application::get().get_window().get_native_window());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { static_cast<float>(xpos), static_cast<float>(ypos) };
	}
}