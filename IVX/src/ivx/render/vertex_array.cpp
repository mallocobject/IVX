#include "ivxpch.h"
#include "ivx/render/vertex_array.h"
#include "backend/opengl/opengl_vertex_array.h"
#include "ivx/core.h"
#include "ivx/render/renderer.h"

namespace ivx {
VertexArray *VertexArray::create() {
    using enum Renderer::API;
    switch (Renderer::get_api()) {
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