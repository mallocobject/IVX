#include "ivxpch.h"
#include "renderer_api.h"

#include "backend/opengl/opengl_renderer_api.h"
#include "ivx/core/base.h"
#include "ivx/renderer/renderer_api.h"

namespace ivx {
RendererAPI::API RendererAPI::api_ = RendererAPI::API::kOpenGL;

Ref<RendererAPI> RendererAPI::create() {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLRendererAPI>();
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
} // namespace ivx