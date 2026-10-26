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
        return make_ref<OpenGLContext<SDL_Window>>(
            reinterpret_cast<SDL_Window *>(window_handle));
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
