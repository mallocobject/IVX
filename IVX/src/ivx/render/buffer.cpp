#include "ivxpch.h"
#include "ivx/render/buffer.h"

#include "backend/opengl/opengl_buffer.h"
#include "ivx/core.h"
#include "renderer.h"

namespace ivx {
VertexBuffer *VertexBuffer::create(float *vertices, uint32_t count) {
    using enum Renderer::API;
    switch (Renderer::get_api()) {
    case kOpenGL:
        return new OpenGLVertexBuffer(vertices, count);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}

IndexBuffer *IndexBuffer::create(uint32_t *indices, uint32_t count) {
    using enum Renderer::API;
    switch (Renderer::get_api()) {
    case kOpenGL:
        return new OpenGLIndexBuffer(indices, count);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
} // namespace ivx