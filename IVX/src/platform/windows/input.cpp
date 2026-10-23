#include "ivxpch.h"
#include "ivx/core/application.h"
#include "ivx/core/input.h"
#include <cstdint>

#if defined(IVX_WINDOW_SDL3)
#include <SDL3/SDL.h>
#else
#include <GLFW/glfw3.h>
#endif

namespace ivx {
#if defined(IVX_WINDOW_SDL3)
namespace {
// ivx::KeyCode uses the GLFW numbering; polling needs the reverse mapping back
// to SDL scancodes.
SDL_Scancode scancode_from_key(KeyCode key) {
    using enum KeyCode;

    const auto value = static_cast<int>(key);
    if (value >= static_cast<int>(A) && value <= static_cast<int>(Z)) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (value - 'A'));
    }
    if (value >= static_cast<int>(D1) && value <= static_cast<int>(D9)) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_1 + (value - '1'));
    }
    if (value >= static_cast<int>(F1) && value <= static_cast<int>(F12)) {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_F1 +
                                         (value - static_cast<int>(F1)));
    }

    switch (key) {
    case D0:
        return SDL_SCANCODE_0;
    case Space:
        return SDL_SCANCODE_SPACE;
    case Apostrophe:
        return SDL_SCANCODE_APOSTROPHE;
    case Comma:
        return SDL_SCANCODE_COMMA;
    case Minus:
        return SDL_SCANCODE_MINUS;
    case Period:
        return SDL_SCANCODE_PERIOD;
    case Slash:
        return SDL_SCANCODE_SLASH;
    case Semicolon:
        return SDL_SCANCODE_SEMICOLON;
    case Equal:
        return SDL_SCANCODE_EQUALS;
    case LeftBracket:
        return SDL_SCANCODE_LEFTBRACKET;
    case Backslash:
        return SDL_SCANCODE_BACKSLASH;
    case RightBracket:
        return SDL_SCANCODE_RIGHTBRACKET;
    case GraveAccent:
        return SDL_SCANCODE_GRAVE;
    case Escape:
        return SDL_SCANCODE_ESCAPE;
    case Enter:
        return SDL_SCANCODE_RETURN;
    case Tab:
        return SDL_SCANCODE_TAB;
    case Backspace:
        return SDL_SCANCODE_BACKSPACE;
    case Insert:
        return SDL_SCANCODE_INSERT;
    case Delete:
        return SDL_SCANCODE_DELETE;
    case Right:
        return SDL_SCANCODE_RIGHT;
    case Left:
        return SDL_SCANCODE_LEFT;
    case Down:
        return SDL_SCANCODE_DOWN;
    case Up:
        return SDL_SCANCODE_UP;
    case PageUp:
        return SDL_SCANCODE_PAGEUP;
    case PageDown:
        return SDL_SCANCODE_PAGEDOWN;
    case Home:
        return SDL_SCANCODE_HOME;
    case End:
        return SDL_SCANCODE_END;
    case CapsLock:
        return SDL_SCANCODE_CAPSLOCK;
    case ScrollLock:
        return SDL_SCANCODE_SCROLLLOCK;
    case NumLock:
        return SDL_SCANCODE_NUMLOCKCLEAR;
    case PrintScreen:
        return SDL_SCANCODE_PRINTSCREEN;
    case Pause:
        return SDL_SCANCODE_PAUSE;
    case KP0:
        return SDL_SCANCODE_KP_0;
    case KPDecimal:
        return SDL_SCANCODE_KP_PERIOD;
    case KPDivide:
        return SDL_SCANCODE_KP_DIVIDE;
    case KPMultiply:
        return SDL_SCANCODE_KP_MULTIPLY;
    case KPSubtract:
        return SDL_SCANCODE_KP_MINUS;
    case KPAdd:
        return SDL_SCANCODE_KP_PLUS;
    case KPEnter:
        return SDL_SCANCODE_KP_ENTER;
    case KPEqual:
        return SDL_SCANCODE_KP_EQUALS;
    case LeftShift:
        return SDL_SCANCODE_LSHIFT;
    case LeftControl:
        return SDL_SCANCODE_LCTRL;
    case LeftAlt:
        return SDL_SCANCODE_LALT;
    case LeftSuper:
        return SDL_SCANCODE_LGUI;
    case RightShift:
        return SDL_SCANCODE_RSHIFT;
    case RightControl:
        return SDL_SCANCODE_RCTRL;
    case RightAlt:
        return SDL_SCANCODE_RALT;
    case RightSuper:
        return SDL_SCANCODE_RGUI;
    case Menu:
        return SDL_SCANCODE_APPLICATION;
    default:
        return SDL_SCANCODE_UNKNOWN;
    }
}

SDL_MouseButtonFlags mask_from_mouse_code(MouseCode button) {
    using enum MouseCode;
    switch (button) {
    case ButtonLeft:
        return SDL_BUTTON_LMASK;
    case ButtonRight:
        return SDL_BUTTON_RMASK;
    case ButtonMiddle:
        return SDL_BUTTON_MMASK;
    case Button3:
        return SDL_BUTTON_X1MASK;
    case Button4:
        return SDL_BUTTON_X2MASK;
    default:
        return 0;
    }
}
} // namespace

bool Input::is_key_pressed(KeyCode key) {
    const SDL_Scancode scancode = scancode_from_key(key);
    if (scancode == SDL_SCANCODE_UNKNOWN) {
        return false;
    }

    int count = 0;
    const bool *state = SDL_GetKeyboardState(&count);
    if (state == nullptr || scancode >= count) {
        return false;
    }
    return state[scancode];
}

bool Input::is_mouse_button_pressed(MouseCode button) {
    const SDL_MouseButtonFlags mask = mask_from_mouse_code(button);
    if (mask == 0) {
        return false;
    }

    float x = 0.0f, y = 0.0f;
    return (SDL_GetMouseState(&x, &y) & mask) != 0;
}

std::pair<float, float> Input::get_mouse_pos() {
    float x = 0.0f, y = 0.0f;
    SDL_GetMouseState(&x, &y);
    return {x, y};
}
#else
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
#endif
} // namespace ivx
