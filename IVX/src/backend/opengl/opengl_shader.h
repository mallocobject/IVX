#pragma once

#include "ivx/render/shader.h"

namespace ivx {
class OpenGLShader : public Shader {
  public:
    OpenGLShader(const std::string &vs_src, const std::string &fs_src);
    virtual ~OpenGLShader();

    virtual void bind() override;
    virtual void unbind() override;

  private:
    uint32_t program_{0};
};
} // namespace ivx