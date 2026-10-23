#include "ivxpch.h"
#include "graphics_context.h"
#include "ivx/core/base.h"
#include "ivx/renderer/renderer_api.h"

#include "backend/opengl/opengl_context.hpp"

namespace ivx {
Ref<GraphicsContext> GraphicsContext::create(void *window_handle) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
#if defined(IVX_WINDOW_SDL3)
        return make_ref<OpenGLContext<SDL_Window>>(
            reinterpret_cast<SDL_Window *>(window_handle));
#else
        return make_ref<OpenGLContext<GLFWwindow>>(
            reinterpret_cast<GLFWwindow *>(window_handle));
#endif
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx