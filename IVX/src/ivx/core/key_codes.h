#pragma once

#include "mouse_codes.h"
#include <cstdint>
#include <format>
#include <string_view>

namespace ivx {
enum class KeyCode : uint16_t {
    // From glfw3.h
    Space = 32,
    Apostrophe = 39, /* ' */
    Comma = 44,      /* , */
    Minus = 45,      /* - */
    Period = 46,     /* . */
    Slash = 47,      /* / */

    D0 = 48, /* 0 */
    D1 = 49, /* 1 */
    D2 = 50, /* 2 */
    D3 = 51, /* 3 */
    D4 = 52, /* 4 */
    D5 = 53, /* 5 */
    D6 = 54, /* 6 */
    D7 = 55, /* 7 */
    D8 = 56, /* 8 */
    D9 = 57, /* 9 */

    Semicolon = 59, /* ; */
    Equal = 61,     /* = */

    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,

    LeftBracket = 91,  /* [ */
    Backslash = 92,    /* \ */
    RightBracket = 93, /* ] */
    GraveAccent = 96,  /* ` */

    World1 = 161, /* non-US #1 */
    World2 = 162, /* non-US #2 */

    /* Function keys */
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,

    /* Keypad */
    KP0 = 320,
    KP1 = 321,
    KP2 = 322,
    KP3 = 323,
    KP4 = 324,
    KP5 = 325,
    KP6 = 326,
    KP7 = 327,
    KP8 = 328,
    KP9 = 329,
    KPDecimal = 330,
    KPDivide = 331,
    KPMultiply = 332,
    KPSubtract = 333,
    KPAdd = 334,
    KPEnter = 335,
    KPEqual = 336,

    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348
};

constexpr std::string_view to_string(KeyCode code) {
    using enum KeyCode;

    switch (code) {
    case Space:
        return "Space";
    case Apostrophe:
        return "'";
    case Comma:
        return ",";
    case Minus:
        return "-";
    case Period:
        return ".";
    case Slash:
        return "/";

    case D0:
        return "0";
    case D1:
        return "1";
    case D2:
        return "2";
    case D3:
        return "3";
    case D4:
        return "4";
    case D5:
        return "5";
    case D6:
        return "6";
    case D7:
        return "7";
    case D8:
        return "8";
    case D9:
        return "9";

    case Semicolon:
        return ";";
    case Equal:
        return "=";

    case A:
        return "A";
    case B:
        return "B";
    case C:
        return "C";
    case D:
        return "D";
    case E:
        return "E";
    case F:
        return "F";
    case G:
        return "G";
    case H:
        return "H";
    case I:
        return "I";
    case J:
        return "J";
    case K:
        return "K";
    case L:
        return "L";
    case M:
        return "M";
    case N:
        return "N";
    case O:
        return "O";
    case P:
        return "P";
    case Q:
        return "Q";
    case R:
        return "R";
    case S:
        return "S";
    case T:
        return "T";
    case U:
        return "U";
    case V:
        return "V";
    case W:
        return "W";
    case X:
        return "X";
    case Y:
        return "Y";
    case Z:
        return "Z";

    case LeftBracket:
        return "[";
    case Backslash:
        return "\\";
    case RightBracket:
        return "]";
    case GraveAccent:
        return "`";

    case World1:
        return "World1";
    case World2:
        return "World2";

    case Escape:
        return "Escape";
    case Enter:
        return "Enter";
    case Tab:
        return "Tab";
    case Backspace:
        return "Backspace";
    case Insert:
        return "Insert";
    case Delete:
        return "Delete";
    case Right:
        return "Right";
    case Left:
        return "Left";
    case Down:
        return "Down";
    case Up:
        return "Up";
    case PageUp:
        return "PageUp";
    case PageDown:
        return "PageDown";
    case Home:
        return "Home";
    case End:
        return "End";
    case CapsLock:
        return "CapsLock";
    case ScrollLock:
        return "ScrollLock";
    case NumLock:
        return "NumLock";
    case PrintScreen:
        return "PrintScreen";
    case Pause:
        return "Pause";

    case F1:
        return "F1";
    case F2:
        return "F2";
    case F3:
        return "F3";
    case F4:
        return "F4";
    case F5:
        return "F5";
    case F6:
        return "F6";
    case F7:
        return "F7";
    case F8:
        return "F8";
    case F9:
        return "F9";
    case F10:
        return "F10";
    case F11:
        return "F11";
    case F12:
        return "F12";
    case F13:
        return "F13";
    case F14:
        return "F14";
    case F15:
        return "F15";
    case F16:
        return "F16";
    case F17:
        return "F17";
    case F18:
        return "F18";
    case F19:
        return "F19";
    case F20:
        return "F20";
    case F21:
        return "F21";
    case F22:
        return "F22";
    case F23:
        return "F23";
    case F24:
        return "F24";
    case F25:
        return "F25";

    case KP0:
        return "KP0";
    case KP1:
        return "KP1";
    case KP2:
        return "KP2";
    case KP3:
        return "KP3";
    case KP4:
        return "KP4";
    case KP5:
        return "KP5";
    case KP6:
        return "KP6";
    case KP7:
        return "KP7";
    case KP8:
        return "KP8";
    case KP9:
        return "KP9";
    case KPDecimal:
        return "KPDecimal";
    case KPDivide:
        return "KPDivide";
    case KPMultiply:
        return "KPMultiply";
    case KPSubtract:
        return "KPSubtract";
    case KPAdd:
        return "KPAdd";
    case KPEnter:
        return "KPEnter";
    case KPEqual:
        return "KPEqual";

    case LeftShift:
        return "LeftShift";
    case LeftControl:
        return "LeftControl";
    case LeftAlt:
        return "LeftAlt";
    case LeftSuper:
        return "LeftSuper";
    case RightShift:
        return "RightShift";
    case RightControl:
        return "RightControl";
    case RightAlt:
        return "RightAlt";
    case RightSuper:
        return "RightSuper";
    case Menu:
        return "Menu";
    }
    return "Unknown";
}

} // namespace ivx

template <>
struct std::formatter<ivx::KeyCode> : std::formatter<std::string_view> {
    auto format(ivx::KeyCode code, format_context &ctx) const {
        return formatter<string_view>::format(ivx::to_string(code), ctx);
    }
};

// From glfw3.h
#define IVX_KEY_SPACE ::ivx::KeyCode::Space
#define IVX_KEY_APOSTROPHE ::ivx::KeyCode::Apostrophe /* ' */
#define IVX_KEY_COMMA ::ivx::KeyCode::Comma           /* , */
#define IVX_KEY_MINUS ::ivx::KeyCode::Minus           /* - */
#define IVX_KEY_PERIOD ::ivx::KeyCode::Period         /* . */
#define IVX_KEY_SLASH ::ivx::KeyCode::Slash           /* / */
#define IVX_KEY_0 ::ivx::KeyCode::D0
#define IVX_KEY_1 ::ivx::KeyCode::D1
#define IVX_KEY_2 ::ivx::KeyCode::D2
#define IVX_KEY_3 ::ivx::KeyCode::D3
#define IVX_KEY_4 ::ivx::KeyCode::D4
#define IVX_KEY_5 ::ivx::KeyCode::D5
#define IVX_KEY_6 ::ivx::KeyCode::D6
#define IVX_KEY_7 ::ivx::KeyCode::D7
#define IVX_KEY_8 ::ivx::KeyCode::D8
#define IVX_KEY_9 ::ivx::KeyCode::D9
#define IVX_KEY_SEMICOLON ::ivx::KeyCode::Semicolon /* ; */
#define IVX_KEY_EQUAL ::ivx::KeyCode::Equal         /* = */
#define IVX_KEY_A ::ivx::KeyCode::A
#define IVX_KEY_B ::ivx::KeyCode::B
#define IVX_KEY_C ::ivx::KeyCode::C
#define IVX_KEY_D ::ivx::KeyCode::D
#define IVX_KEY_E ::ivx::KeyCode::E
#define IVX_KEY_F ::ivx::KeyCode::F
#define IVX_KEY_G ::ivx::KeyCode::G
#define IVX_KEY_H ::ivx::KeyCode::H
#define IVX_KEY_I ::ivx::KeyCode::I
#define IVX_KEY_J ::ivx::KeyCode::J
#define IVX_KEY_K ::ivx::KeyCode::K
#define IVX_KEY_L ::ivx::KeyCode::L
#define IVX_KEY_M ::ivx::KeyCode::M
#define IVX_KEY_N ::ivx::KeyCode::N
#define IVX_KEY_O ::ivx::KeyCode::O
#define IVX_KEY_P ::ivx::KeyCode::P
#define IVX_KEY_Q ::ivx::KeyCode::Q
#define IVX_KEY_R ::ivx::KeyCode::R
#define IVX_KEY_S ::ivx::KeyCode::S
#define IVX_KEY_T ::ivx::KeyCode::T
#define IVX_KEY_U ::ivx::KeyCode::U
#define IVX_KEY_V ::ivx::KeyCode::V
#define IVX_KEY_W ::ivx::KeyCode::W
#define IVX_KEY_X ::ivx::KeyCode::X
#define IVX_KEY_Y ::ivx::KeyCode::Y
#define IVX_KEY_Z ::ivx::KeyCode::Z
#define IVX_KEY_LEFT_BRACKET ::ivx::KeyCode::LeftBracket   /* [ */
#define IVX_KEY_BACKSLASH ::ivx::KeyCode::Backslash        /* \ */
#define IVX_KEY_RIGHT_BRACKET ::ivx::KeyCode::RightBracket /* ] */
#define IVX_KEY_GRAVE_ACCENT ::ivx::KeyCode::GraveAccent   /* ` */
#define IVX_KEY_WORLD_1 ::ivx::KeyCode::World1             /* non-US #1 */
#define IVX_KEY_WORLD_2 ::ivx::KeyCode::World2             /* non-US #2 */

/* Function keys */
#define IVX_KEY_ESCAPE ::ivx::KeyCode::Escape
#define IVX_KEY_ENTER ::ivx::KeyCode::Enter
#define IVX_KEY_TAB ::ivx::KeyCode::Tab
#define IVX_KEY_BACKSPACE ::ivx::KeyCode::Backspace
#define IVX_KEY_INSERT ::ivx::KeyCode::Insert
#define IVX_KEY_DELETE ::ivx::KeyCode::Delete
#define IVX_KEY_RIGHT ::ivx::KeyCode::Right
#define IVX_KEY_LEFT ::ivx::KeyCode::Left
#define IVX_KEY_DOWN ::ivx::KeyCode::Down
#define IVX_KEY_UP ::ivx::KeyCode::Up
#define IVX_KEY_PAGE_UP ::ivx::KeyCode::PageUp
#define IVX_KEY_PAGE_DOWN ::ivx::KeyCode::PageDown
#define IVX_KEY_HOME ::ivx::KeyCode::Home
#define IVX_KEY_END ::ivx::KeyCode::End
#define IVX_KEY_CAPS_LOCK ::ivx::KeyCode::CapsLock
#define IVX_KEY_SCROLL_LOCK ::ivx::KeyCode::ScrollLock
#define IVX_KEY_NUM_LOCK ::ivx::KeyCode::NumLock
#define IVX_KEY_PRINT_SCREEN ::ivx::KeyCode::PrintScreen
#define IVX_KEY_PAUSE ::ivx::KeyCode::Pause
#define IVX_KEY_F1 ::ivx::KeyCode::F1
#define IVX_KEY_F2 ::ivx::KeyCode::F2
#define IVX_KEY_F3 ::ivx::KeyCode::F3
#define IVX_KEY_F4 ::ivx::KeyCode::F4
#define IVX_KEY_F5 ::ivx::KeyCode::F5
#define IVX_KEY_F6 ::ivx::KeyCode::F6
#define IVX_KEY_F7 ::ivx::KeyCode::F7
#define IVX_KEY_F8 ::ivx::KeyCode::F8
#define IVX_KEY_F9 ::ivx::KeyCode::F9
#define IVX_KEY_F10 ::ivx::KeyCode::F10
#define IVX_KEY_F11 ::ivx::KeyCode::F11
#define IVX_KEY_F12 ::ivx::KeyCode::F12
#define IVX_KEY_F13 ::ivx::KeyCode::F13
#define IVX_KEY_F14 ::ivx::KeyCode::F14
#define IVX_KEY_F15 ::ivx::KeyCode::F15
#define IVX_KEY_F16 ::ivx::KeyCode::F16
#define IVX_KEY_F17 ::ivx::KeyCode::F17
#define IVX_KEY_F18 ::ivx::KeyCode::F18
#define IVX_KEY_F19 ::ivx::KeyCode::F19
#define IVX_KEY_F20 ::ivx::KeyCode::F20
#define IVX_KEY_F21 ::ivx::KeyCode::F21
#define IVX_KEY_F22 ::ivx::KeyCode::F22
#define IVX_KEY_F23 ::ivx::KeyCode::F23
#define IVX_KEY_F24 ::ivx::KeyCode::F24
#define IVX_KEY_F25 ::ivx::KeyCode::F25

/* Keypad */
#define IVX_KEY_KP_0 ::ivx::KeyCode::KP0
#define IVX_KEY_KP_1 ::ivx::KeyCode::KP1
#define IVX_KEY_KP_2 ::ivx::KeyCode::KP2
#define IVX_KEY_KP_3 ::ivx::KeyCode::KP3
#define IVX_KEY_KP_4 ::ivx::KeyCode::KP4
#define IVX_KEY_KP_5 ::ivx::KeyCode::KP5
#define IVX_KEY_KP_6 ::ivx::KeyCode::KP6
#define IVX_KEY_KP_7 ::ivx::KeyCode::KP7
#define IVX_KEY_KP_8 ::ivx::KeyCode::KP8
#define IVX_KEY_KP_9 ::ivx::KeyCode::KP9
#define IVX_KEY_KP_DECIMAL ::ivx::KeyCode::KPDecimal
#define IVX_KEY_KP_DIVIDE ::ivx::KeyCode::KPDivide
#define IVX_KEY_KP_MULTIPLY ::ivx::KeyCode::KPMultiply
#define IVX_KEY_KP_SUBTRACT ::ivx::KeyCode::KPSubtract
#define IVX_KEY_KP_ADD ::ivx::KeyCode::KPAdd
#define IVX_KEY_KP_ENTER ::ivx::KeyCode::KPEnter
#define IVX_KEY_KP_EQUAL ::ivx::KeyCode::KPEqual

#define IVX_KEY_LEFT_SHIFT ::ivx::KeyCode::LeftShift
#define IVX_KEY_LEFT_CONTROL ::ivx::KeyCode::LeftControl
#define IVX_KEY_LEFT_ALT ::ivx::KeyCode::LeftAlt
#define IVX_KEY_LEFT_SUPER ::ivx::KeyCode::LeftSuper
#define IVX_KEY_RIGHT_SHIFT ::ivx::KeyCode::RightShift
#define IVX_KEY_RIGHT_CONTROL ::ivx::KeyCode::RightControl
#define IVX_KEY_RIGHT_ALT ::ivx::KeyCode::RightAlt
#define IVX_KEY_RIGHT_SUPER ::ivx::KeyCode::RightSuper
#define IVX_KEY_MENU ::ivx::KeyCode::Menu
