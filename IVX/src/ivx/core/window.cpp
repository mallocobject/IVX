#include "ivxpch.h"
#include "window.h"

#if defined(IVX_PLATFORM_WINDOWS)
#if defined(IVX_WINDOW_SDL3)
#include "platform/windows/sdl_window.h"
#else
#include "platform/windows/glfw_window.h"
#endif
#endif

namespace ivx {
Ref<Window> Window::create(const WindowProps &props) {
#if defined(IVX_PLATFORM_WINDOWS)
#if defined(IVX_WINDOW_SDL3)
    return make_ref<SDLWindow>(props);
#else
    return make_ref<GLFWWindow>(props);
#endif
#else
    HZ_CORE_ASSERT(false, "Unknown platform!");
    return nullptr;
#endif
}
} // namespace ivx