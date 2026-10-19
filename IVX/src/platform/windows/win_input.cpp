#include "ivxpch.h"
#include "win_input.h"
#include "ivx/core/application.h"
#include <GLFW/glfw3.h>
#include <cstdint>

namespace ivx {
Input *Input::instance_ = new WinInput;

bool WinInput::is_key_pressed_impl(KeyCode key) {
    auto window = reinterpret_cast<GLFWwindow *>(
        Application::get().get_window().get_native_window());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool WinInput::is_mouse_button_pressed_impl(MouseCode button) {
    auto window = reinterpret_cast<GLFWwindow *>(
        Application::get().get_window().get_native_window());
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

std::pair<float, float> WinInput::get_mouse_pos_impl() {
    auto window = reinterpret_cast<GLFWwindow *>(
        Application::get().get_window().get_native_window());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    return {static_cast<float>(xpos), static_cast<float>(ypos)};
}
} // namespace ivx
