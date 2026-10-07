#include "ivxpch.h"
#include "ivx/render/shader.h"

#include "backend/opengl/opengl_shader.h"
#include "ivx/core.h"
#include "renderer.h"

namespace ivx {
Shader *Shader::create(const std::string &vs_src, const std::string &fs_src) {
    using enum Renderer::API;
    switch (Renderer::get_api()) {
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