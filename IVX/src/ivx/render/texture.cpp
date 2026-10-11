#include "ivxpch.h"
#include "texture.h"

#include "backend/opengl/opengl_texture.h"
#include "ivx/core.h"
#include "renderer_api.h"

namespace ivx {
Ref<Texture2D> Texture2D::create(const std::string &path) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLTexture2D>(path);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
} // namespace ivx