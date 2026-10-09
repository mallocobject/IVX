#include "ivxpch.h"
#include "vertex_array.h"
#include "backend/opengl/opengl_vertex_array.h"
#include "ivx/core.h"
#include "renderer_api.h"

namespace ivx {
VertexArray *VertexArray::create() {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return new OpenGLVertexArray;
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
} // namespace ivx