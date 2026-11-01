#include "ivxpch.h"
#include "uniform_buffer.h"

#include "backend/opengl/opengl_uniform_buffer.h"
#include "ivx/core/base.h"
#include "ivx/render/renderer_api.h"

namespace ivx {
Ref<UniformBuffer> UniformBuffer::create(uint32_t size, uint32_t binding) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLUniformBuffer>(size, binding);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
