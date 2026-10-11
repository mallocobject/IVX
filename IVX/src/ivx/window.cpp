#include "ivxpch.h"
#include "window.h"

#include "platform/windows/win_window.h"

namespace ivx {
Ref<Window> Window::create(const WindowProps &props) {
    return make_ref<WinWindow>(props);
}
} // namespace ivx