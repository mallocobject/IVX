#include "ivxpch.h"
#include "ivx/core/application.h"
#include "ivx/core/input.h"
#include <GLFW/glfw3.h>
#include <cstdint>

namespace ivx {
bool Input::is_key_pressed(KeyCode key) {
    auto window = reinterpret_cast<GLFWwindow *>(
        Application::get().get_window().get_native_window());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::is_mouse_button_pressed(MouseCode button) {
    auto window = reinterpret_cast<GLFWwindow *>(
        Application::get().get_window().get_native_window());
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

std::pair<float, float> Input::get_mouse_pos() {
    auto window = reinterpret_cast<GLFWwindow *>(
        Application::get().get_window().get_native_window());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    return {static_cast<float>(xpos), static_cast<float>(ypos)};
}
} // namespace ivx
