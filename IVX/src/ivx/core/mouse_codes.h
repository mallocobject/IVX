#pragma once

#include <cstdint>
#include <format>
#include <iostream>
#include <string_view>

namespace ivx {
enum class MouseCode : uint8_t {
    // From glfw3.h
    Button0 = 0,
    Button1 = 1,
    Button2 = 2,
    Button3 = 3,
    Button4 = 4,
    Button5 = 5,
    Button6 = 6,
    Button7 = 7,

    ButtonLast = Button7,
    ButtonLeft = Button0,
    ButtonRight = Button1,
    ButtonMiddle = Button2
};

constexpr std::string_view to_string(MouseCode code) {
    using enum MouseCode;
    switch (code) {
    case Button0:
        return "Left";
    case Button1:
        return "Right";
    case Button2:
        return "Middle";
    case Button3:
        return "Button3";
    case Button4:
        return "Button4";
    case Button5:
        return "Button5";
    case Button6:
        return "Button6";
    case Button7:
        return "Button7";
    }
    return "Unknown";
}

} // namespace ivx

template <>
struct std::formatter<ivx::MouseCode> : std::formatter<std::string_view> {
    auto format(ivx::MouseCode code, format_context &ctx) const {
        return formatter<string_view>::format(ivx::to_string(code), ctx);
    }
};

#define IVX_MOUSE_BUTTON_0 ::ivx::MouseCode::Button0
#define IVX_MOUSE_BUTTON_1 ::ivx::MouseCode::Button1
#define IVX_MOUSE_BUTTON_2 ::ivx::MouseCode::Button2
#define IVX_MOUSE_BUTTON_3 ::ivx::MouseCode::Button3
#define IVX_MOUSE_BUTTON_4 ::ivx::MouseCode::Button4
#define IVX_MOUSE_BUTTON_5 ::ivx::MouseCode::Button5
#define IVX_MOUSE_BUTTON_6 ::ivx::MouseCode::Button6
#define IVX_MOUSE_BUTTON_7 ::ivx::MouseCode::Button7
#define IVX_MOUSE_BUTTON_LAST ::ivx::MouseCode::ButtonLast
#define IVX_MOUSE_BUTTON_LEFT ::ivx::MouseCode::ButtonLeft
#define IVX_MOUSE_BUTTON_RIGHT ::ivx::MouseCode::ButtonRight
#define IVX_MOUSE_BUTTON_MIDDLE ::ivx::MouseCode::ButtonMiddle