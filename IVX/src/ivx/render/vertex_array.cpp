#include "ivxpch.h"
#include "vertex_array.h"
#include "backend/opengl/opengl_vertex_array.h"
#include "ivx/core/base.h"
#include "ivx/render/renderer_api.h"

namespace ivx {
Ref<VertexArray> VertexArray::create() {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLVertexArray>();
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
