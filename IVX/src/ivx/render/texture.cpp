#include "ivxpch.h"
#include "texture.h"

#include "backend/opengl/opengl_texture.h"
#include "ivx/core/base.h"
#include "ivx/render/renderer_api.h"

namespace ivx {
Ref<Texture2D> Texture2D::create(uint32_t width, uint32_t height) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLTexture2D>(width, height);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}

Ref<Texture2D> Texture2D::create(const std::filesystem::path &filepath) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLTexture2D>(filepath);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "Unknown backend!");
        return nullptr;
    }
    }
}
} // namespace ivx
