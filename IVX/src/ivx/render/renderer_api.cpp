#include "ivxpch.h"
#include "renderer_api.h"

#include "backend/opengl/opengl_renderer_api.h"
#include "ivx/core/base.h"
#include "ivx/render/renderer_api.h"

namespace ivx {
RendererAPI::API RendererAPI::api_ = RendererAPI::API::IVX_OpenGL;

Ref<RendererAPI> RendererAPI::create() {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case IVX_OpenGL:
        return make_ref<OpenGLRendererAPI>();
    case IVX_None:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
