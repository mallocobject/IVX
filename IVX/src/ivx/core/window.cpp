#include "ivxpch.h"
#include "window.h"

#if defined(IVX_PLATFORM_WINDOWS)
#include "platform/windows/win_window.h"
#endif

namespace ivx {
Ref<Window> Window::create(const WindowProps &props) {
#ifdef IVX_PLATFORM_WINDOWS
    return make_ref<WinWindow>(props);
#else
    HZ_CORE_ASSERT(false, "Unknown platform!");
    return nullptr;
#endif
}
} // namespace ivx