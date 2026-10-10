#pragma once

#include "ivx/render/shader.h"
#include <cstdint>
#include <glad/glad.h>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace ivx {
class OpenGLShader : public Shader {
  public:
    OpenGLShader(const std::string &vs_src, const std::string &fs_src);
    virtual ~OpenGLShader();

    virtual void bind() override;
    virtual void unbind() override;

    void set(const std::string &name, bool val) override;
    void set(const std::string &name, float val) override;
    void set(const std::string &name, const glm::vec2 &val) override;
    void set(const std::string &name, const glm::vec3 &val) override;
    void set(const std::string &name, const glm::vec4 &val) override;
    void set(const std::string &name, const glm::ivec2 &val) override;
    void set(const std::string &name, const glm::ivec3 &val) override;
    void set(const std::string &name, const glm::ivec4 &val) override;
    void set(const std::string &name, const glm::mat3 &val) override;
    void set(const std::string &name, const glm::mat4 &val) override;

  private:
    GLint location(const std::string &name);

    uint32_t program_{0};
    std::unordered_map<std::string, GLint> uniform_mp_;
};
} // namespace ivx