#pragma once

#include "ivx/render/shader.h"
#include <cstdint>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace ivx {
class OpenGLShader : public Shader {
  public:
    OpenGLShader(const std::string &vs_src,
                 const std::string &fs_src,
                 const std::string &label);
    OpenGLShader(const std::filesystem::path &filepath,
                 const std::string &label = {});
    virtual ~OpenGLShader();

    virtual void bind() override;
    virtual void unbind() override;
    virtual const std::string &get_label() const override {
        return label_;
    }

    virtual void set(const std::string &name, bool val) override;
    virtual void set(const std::string &name, float val) override;
    virtual void set(const std::string &name, int val) override;
    virtual void set(const std::string &name, uint32_t val) override;
    virtual void set(const std::string &name, const glm::vec2 &val) override;
    virtual void set(const std::string &name, const glm::vec3 &val) override;
    virtual void set(const std::string &name, const glm::vec4 &val) override;
    virtual void set(const std::string &name, const glm::ivec2 &val) override;
    virtual void set(const std::string &name, const glm::ivec3 &val) override;
    virtual void set(const std::string &name, const glm::ivec4 &val) override;
    virtual void set(const std::string &name, const glm::mat3 &val) override;
    virtual void set(const std::string &name, const glm::mat4 &val) override;
    virtual void
    set(const std::string &name, const float *vals, uint32_t count) override;
    virtual void
    set(const std::string &name, const int *vals, uint32_t count) override;

  private:
    int location(const std::string &name);

    uint32_t program_{0};
    std::unordered_map<std::string, int> uniform_mp_;
    std::string label_;
};
} // namespace ivx
