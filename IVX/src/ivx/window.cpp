#include "ivxpch.h"
#include "window.h"

#include "platform/windows/win_window.h"

namespace ivx {
Window *Window::create(const WindowProps &props) {
    return new WinWindow(props);
}
} // namespace ivx