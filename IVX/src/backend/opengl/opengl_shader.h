#pragma once

#include "ivx/render/shader.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>

namespace ivx {
class OpenGLShader : public Shader {
  public:
    OpenGLShader(const std::string &vs_src, const std::string &fs_src);
    virtual ~OpenGLShader();

    virtual void bind() override;
    virtual void unbind() override;

    void set(const std::string &name, bool val) const override;
    void set(const std::string &name, float val) const override;
    void set(const std::string &name, const glm::vec2 &val) const override;
    void set(const std::string &name, const glm::vec3 &val) const override;
    void set(const std::string &name, const glm::vec4 &val) const override;
    void set(const std::string &name, const glm::ivec2 &val) const override;
    void set(const std::string &name, const glm::ivec3 &val) const override;
    void set(const std::string &name, const glm::ivec4 &val) const override;
    void set(const std::string &name, const glm::mat3 &val) const override;
    void set(const std::string &name, const glm::mat4 &val) const override;

  private:
    int location(const std::string &name) const;

    uint32_t program_{0};
};
} // namespace ivx