#include "ivxpch.h"
#include "sdl_window.h"
#include "ivx/event/application_event.h"
#include "ivx/event/key_event.h"
#include "ivx/event/mouse_event.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace ivx {
namespace {
uint8_t g_sdl_initialized = 0;

// ivx::KeyCode mirrors the GLFW key values (uppercase letters, 256+ for the
// special keys), while SDL reports scancodes / lowercase keycodes. Physical keys
// are mapped through the scancode so the result does not depend on the layout.
KeyCode key_from_scancode(SDL_Scancode sc) {
    using enum KeyCode;

    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        return static_cast<KeyCode>('A' + (sc - SDL_SCANCODE_A));
    }
    if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) {
        return static_cast<KeyCode>('1' + (sc - SDL_SCANCODE_1));
    }
    if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F12) {
        return static_cast<KeyCode>(static_cast<int>(F1) +
                                    (sc - SDL_SCANCODE_F1));
    }
    if (sc >= SDL_SCANCODE_KP_1 && sc <= SDL_SCANCODE_KP_9) {
        return static_cast<KeyCode>(static_cast<int>(KP1) +
                                    (sc - SDL_SCANCODE_KP_1));
    }

    switch (sc) {
    case SDL_SCANCODE_0:
        return D0;
    case SDL_SCANCODE_SPACE:
        return Space;
    case SDL_SCANCODE_APOSTROPHE:
        return Apostrophe;
    case SDL_SCANCODE_COMMA:
        return Comma;
    case SDL_SCANCODE_MINUS:
        return Minus;
    case SDL_SCANCODE_PERIOD:
        return Period;
    case SDL_SCANCODE_SLASH:
        return Slash;
    case SDL_SCANCODE_SEMICOLON:
        return Semicolon;
    case SDL_SCANCODE_EQUALS:
        return Equal;
    case SDL_SCANCODE_LEFTBRACKET:
        return LeftBracket;
    case SDL_SCANCODE_BACKSLASH:
        return Backslash;
    case SDL_SCANCODE_RIGHTBRACKET:
        return RightBracket;
    case SDL_SCANCODE_GRAVE:
        return GraveAccent;

    case SDL_SCANCODE_ESCAPE:
        return Escape;
    case SDL_SCANCODE_RETURN:
        return Enter;
    case SDL_SCANCODE_TAB:
        return Tab;
    case SDL_SCANCODE_BACKSPACE:
        return Backspace;
    case SDL_SCANCODE_INSERT:
        return Insert;
    case SDL_SCANCODE_DELETE:
        return Delete;
    case SDL_SCANCODE_RIGHT:
        return Right;
    case SDL_SCANCODE_LEFT:
        return Left;
    case SDL_SCANCODE_DOWN:
        return Down;
    case SDL_SCANCODE_UP:
        return Up;
    case SDL_SCANCODE_PAGEUP:
        return PageUp;
    case SDL_SCANCODE_PAGEDOWN:
        return PageDown;
    case SDL_SCANCODE_HOME:
        return Home;
    case SDL_SCANCODE_END:
        return End;
    case SDL_SCANCODE_CAPSLOCK:
        return CapsLock;
    case SDL_SCANCODE_SCROLLLOCK:
        return ScrollLock;
    case SDL_SCANCODE_NUMLOCKCLEAR:
        return NumLock;
    case SDL_SCANCODE_PRINTSCREEN:
        return PrintScreen;
    case SDL_SCANCODE_PAUSE:
        return Pause;

    case SDL_SCANCODE_KP_0:
        return KP0;
    case SDL_SCANCODE_KP_PERIOD:
        return KPDecimal;
    case SDL_SCANCODE_KP_DIVIDE:
        return KPDivide;
    case SDL_SCANCODE_KP_MULTIPLY:
        return KPMultiply;
    case SDL_SCANCODE_KP_MINUS:
        return KPSubtract;
    case SDL_SCANCODE_KP_PLUS:
        return KPAdd;
    case SDL_SCANCODE_KP_ENTER:
        return KPEnter;
    case SDL_SCANCODE_KP_EQUALS:
        return KPEqual;

    case SDL_SCANCODE_LSHIFT:
        return LeftShift;
    case SDL_SCANCODE_LCTRL:
        return LeftControl;
    case SDL_SCANCODE_LALT:
        return LeftAlt;
    case SDL_SCANCODE_LGUI:
        return LeftSuper;
    case SDL_SCANCODE_RSHIFT:
        return RightShift;
    case SDL_SCANCODE_RCTRL:
        return RightControl;
    case SDL_SCANCODE_RALT:
        return RightAlt;
    case SDL_SCANCODE_RGUI:
        return RightSuper;
    case SDL_SCANCODE_APPLICATION:
        return Menu;
    default:
        break;
    }

    return static_cast<KeyCode>(0);
}

// SDL reports mouse buttons as SDL_BUTTON_* (1 = left, 2 = middle, 3 = right),
// ivx::MouseCode follows GLFW (0 = left, 1 = right, 2 = middle), so middle and
// right have to be swapped rather than shifted by one.
MouseCode mouse_code_from_sdl(uint8_t button) {
    using enum MouseCode;
    switch (button) {
    case SDL_BUTTON_LEFT:
        return ButtonLeft;
    case SDL_BUTTON_RIGHT:
        return ButtonRight;
    case SDL_BUTTON_MIDDLE:
        return ButtonMiddle;
    case SDL_BUTTON_X1:
        return Button3;
    case SDL_BUTTON_X2:
        return Button4;
    default:
        return static_cast<MouseCode>(button - 1);
    }
}

// SDL_EVENT_TEXT_INPUT delivers UTF-8, ivx::KeyTypedEvent carries one codepoint.
std::vector<uint32_t> decode_utf8(std::string_view text) {
    std::vector<uint32_t> out;
    out.reserve(text.size());

    for (size_t i = 0; i < text.size();) {
        const auto c = static_cast<uint8_t>(text[i]);
        uint32_t cp = 0;
        size_t len = 1;

        if (c < 0x80) {
            cp = c;
        } else if ((c & 0xE0) == 0xC0) {
            cp = c & 0x1F;
            len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            cp = c & 0x0F;
            len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            cp = c & 0x07;
            len = 4;
        } else {
            ++i; // invalid lead byte, skip it
            continue;
        }

        if (i + len > text.size()) {
            break;
        }
        for (size_t k = 1; k < len; ++k) {
            cp = (cp << 6) | (static_cast<uint8_t>(text[i + k]) & 0x3F);
        }

        out.push_back(cp);
        i += len;
    }
    return out;
}
} // namespace

SDLWindow::SDLWindow(const WindowProps &props) {
    IVX_PROFILE_FUNCTION();

    init(props);
}

SDLWindow::~SDLWindow() {
    IVX_PROFILE_FUNCTION();

    shutdown();
}

void SDLWindow::init(const WindowProps &props) {
    IVX_PROFILE_FUNCTION();

    IVX_CORE_INFO(
        "Creating window {} ({}, {})", props.title, props.width, props.height);

    data_.title = props.title;
    data_.width = props.width;
    data_.height = props.height;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        IVX_CORE_ASSERT(false, "SDL_Init failed: {}", SDL_GetError());
    }
    ++g_sdl_initialized;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
#if defined(IVX_CONFIG_DEBUG)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window_handle_ = SDL_CreateWindow(props.title.c_str(),
                                      static_cast<int>(props.width),
                                      static_cast<int>(props.height),
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_HIGH_PIXEL_DENSITY);
    IVX_CORE_ASSERT(
        window_handle_, "SDL_CreateWindow failed: {}", SDL_GetError());

    context_ = GraphicsContext::create(window_handle_);
    context_->init();
    set_v_sync(true);

    // GL viewport / framebuffers work in pixels, which differ from the window
    // size in points on high-DPI displays.
    int pw = 0, ph = 0;
    SDL_GetWindowSizeInPixels(window_handle_, &pw, &ph);
    data_.width = static_cast<uint32_t>(pw);
    data_.height = static_cast<uint32_t>(ph);
}

void SDLWindow::shutdown() {
    IVX_PROFILE_FUNCTION();

    // The GL context must go away before the window it belongs to.
    context_.reset();

    if (window_handle_ != nullptr) {
        SDL_DestroyWindow(window_handle_);
        window_handle_ = nullptr;
    }

    if (g_sdl_initialized > 0 && --g_sdl_initialized == 0) {
        SDL_Quit();
    }
}

void SDLWindow::on_update() {
    IVX_PROFILE_FUNCTION();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (native_cb_) { // Feed ImGui first (it maintains its own io queue)
            native_cb_(&e);
        }
        process_event(e); // Translate it again into engine Event
    }

    context_->swap_buffer();
}

void SDLWindow::set_v_sync(bool enabled) {
    IVX_PROFILE_FUNCTION();

    SDL_GL_SetSwapInterval(enabled ? 1 : 0);
    data_.v_sync = enabled;
}

void SDLWindow::process_event(const SDL_Event &e) {
    auto emit = [this](Event &ev) {
        if (data_.event_callback) {
            data_.event_callback(ev);
        }
    };

    switch (e.type) {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
        WindowCloseEvent ev;
        emit(ev);
        break;
    }
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
        int pw = 0, ph = 0;
        SDL_GetWindowSizeInPixels(window_handle_, &pw, &ph);
        if (static_cast<uint32_t>(pw) == data_.width &&
            static_cast<uint32_t>(ph) == data_.height) {
            break;
        }
        data_.width = static_cast<uint32_t>(pw);
        data_.height = static_cast<uint32_t>(ph);
        WindowResizeEvent ev(pw, ph);
        emit(ev);
        break;
    }
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
        const KeyCode key = key_from_scancode(e.key.scancode);
        if (e.type == SDL_EVENT_KEY_DOWN) {
            KeyPressedEvent ev(key, e.key.repeat);
            emit(ev);
        } else {
            KeyReleasedEvent ev(key);
            emit(ev);
        }
        break;
    }
    case SDL_EVENT_TEXT_INPUT: {
        for (uint32_t cp : decode_utf8(e.text.text)) {
            KeyTypedEvent ev(static_cast<KeyCode>(cp));
            emit(ev);
        }
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        const MouseCode button = mouse_code_from_sdl(e.button.button);
        if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            MouseButtonPressedEvent ev(button);
            emit(ev);
        } else {
            MouseButtonReleasedEvent ev(button);
            emit(ev);
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
        // SDL_MOUSEWHEEL_FLIPPED (touchpads / natural scrolling) reports the
        // values inverted; normalize them so zoom always follows the wheel.
        float x = e.wheel.x;
        float y = e.wheel.y;
        if (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
            x = -x;
            y = -y;
        }
        MouseScrolledEvent ev(x, y);
        emit(ev);
        break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        MouseMovedEvent ev(e.motion.x, e.motion.y);
        emit(ev);
        break;
    }
    case SDL_EVENT_WINDOW_FOCUS_GAINED: {
        WindowFocusEvent ev(true);
        emit(ev);
        break;
    }
    case SDL_EVENT_WINDOW_FOCUS_LOST: {
        WindowFocusEvent ev(false);
        emit(ev);
        break;
    }
    default:
        break;
    }
}
} // namespace ivx
