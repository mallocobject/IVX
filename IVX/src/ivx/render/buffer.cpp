#include "ivxpch.h"
#include "ivx/render/buffer.h"

#include "backend/opengl/opengl_buffer.h"
#include "buffer.h"
#include "ivx/core.h"
#include "renderer.h"

namespace ivx {
BufferElement::BufferElement(ShaderDataType type,
                             const std::string &name,
                             bool normalized)
    : type(type), name(name), size(size_of(type)), offset(0),
      normalized(normalized) {
}

BufferLayout::BufferLayout(const std::initializer_list<BufferElement> &elements)
    : elements_(elements) {
    calc_stride();
}

void BufferLayout::calc_stride() {
    IVX_CORE_ASSERT(stride_ == 0, "Stride must be initialized with 0!");

    for (auto &&element : elements_) {
        element.offset = stride_;
        stride_ += element.size;
    }
}

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