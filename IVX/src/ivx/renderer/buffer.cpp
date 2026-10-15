#include "ivxpch.h"
#include "buffer.h"

#include "backend/opengl/opengl_buffer.h"
#include "buffer.h"
#include "ivx/core/base.h"
#include "ivx/renderer/renderer_api.h"

namespace ivx {
BufferElement::BufferElement(const std::string &name,
                             ShaderDataType type,
                             bool normalized)
    : name(name), type(type), size(size_of(type)), offset(0),
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

Ref<VertexBuffer> VertexBuffer::create(float *vertices, uint32_t count) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLVertexBuffer>(vertices, count);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}

Ref<IndexBuffer> IndexBuffer::create(uint32_t *indices, uint32_t count) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLIndexBuffer>(indices, count);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
