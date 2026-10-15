#include "ivxpch.h"
#include "shader.h"

#include "backend/opengl/opengl_shader.h"
#include "ivx/core/base.h"
#include "ivx/renderer/renderer_api.h"

namespace ivx {
Ref<Shader> Shader::create(const std::string &vs_src,
                           const std::string &fs_src,
                           const std::string &label) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLShader>(vs_src, fs_src, label);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}
Ref<Shader> Shader::create(const std::filesystem::path &filepath,
                           const std::string &label) {
    using enum RendererAPI::API;
    switch (RendererAPI::get_api()) {
    case kOpenGL:
        return make_ref<OpenGLShader>(filepath, label);
    case kNone:
    default: {
        IVX_CORE_ASSERT(false, "IVX is currently supported this backend!");
        return nullptr;
    }
    }
}

void ShaderLibrary::add(const Ref<Shader> &shader) {
    IVX_CORE_ASSERT(!shaders_.contains(shader->get_label()),
                    "Shader already exists!");
    shaders_[shader->get_label()] = shader;
}

Ref<Shader> ShaderLibrary::load(const std::filesystem::path &filepath,
                                const std::string &label) {
    auto shader = Shader::create(filepath, label);
    add(shader);
    return shader;
}

Ref<Shader> ShaderLibrary::get(const std::string &name) {
    if (auto it = shaders_.find(name); it != shaders_.end()) {
        return shaders_[name];
    }

    IVX_CORE_ASSERT(false, "{} not found", name);
    return nullptr;
}
} // namespace ivx
