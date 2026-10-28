#include "ivxpch.h"
#include "frame_buffer.h"
#include "ivx/render/renderer_api.h"

#include "backend/opengl/opengl_frame_buffer.h"

namespace ivx {
Ref<FrameBuffer> FrameBuffer::create(const FrameBufferSpecification &spec) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLFrameBuffer>(spec);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
