#include "ivxpch.h"
#include "window.h"

#if defined(IVX_PLATFORM_WINDOWS)
#include "platform/windows/sdl_window.h"
#endif

namespace ivx {
Ref<Window> Window::create(const WindowProps &props) {
#if defined(IVX_PLATFORM_WINDOWS)
    return make_ref<SDLWindow>(props);
#else
    IVX_CORE_ASSERT(false, "Unknown platform!");
    return nullptr;
#endif
}
} // namespace ivx
