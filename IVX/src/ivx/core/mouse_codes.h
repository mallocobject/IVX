#pragma once

#include <cstdint>
#include <format>
#include <iostream>
#include <string_view>

namespace ivx {
enum class MouseCode : uint8_t {
    // IVX mouse codes, independent of any windowing library. Button0 is the
    // left
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
