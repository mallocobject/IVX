#include "ivxpch.h"
#include "shader.h"

#include "backend/opengl/opengl_shader.h"
#include "ivx/core.h"
#include "renderer_api.h"

namespace ivx {
Shader *Shader::create(const std::string &vs_src, const std::string &fs_src) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return new OpenGLShader(vs_src, fs_src);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
} // namespace ivx