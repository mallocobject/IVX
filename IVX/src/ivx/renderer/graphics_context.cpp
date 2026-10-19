#include "ivxpch.h"
#include "graphics_context.h"
#include "ivx/core/base.h"
#include "ivx/renderer/renderer_api.h"

#include "backend/opengl/opengl_context.h"

namespace ivx {
Ref<GraphicsContext> GraphicsContext::create(void *window_handle) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLContext>(
            reinterpret_cast<GLFWwindow *>(window_handle));
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx