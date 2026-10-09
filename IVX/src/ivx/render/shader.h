#pragma once

#include <cstdint>
#include <string>

#include <glm/glm.hpp>

namespace ivx {
class Shader {
  public:
    // Shader(const std::string &vs_src, const std::string &fs_src);
    virtual ~Shader() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    static Shader *create(const std::string &vs_src, const std::string &fs_src);

    virtual void set(const std::string &name, bool val) const = 0;
    virtual void set(const std::string &name, float val) const = 0;

    virtual void set(const std::string &name, const glm::vec2 &val) const = 0;
    virtual void set(const std::string &name, const glm::vec3 &val) const = 0;
    virtual void set(const std::string &name, const glm::vec4 &val) const = 0;

    virtual void set(const std::string &name, const glm::ivec2 &val) const = 0;
    virtual void set(const std::string &name, const glm::ivec3 &val) const = 0;
    virtual void set(const std::string &name, const glm::ivec4 &val) const = 0;

    virtual void set(const std::string &name, const glm::mat3 &val) const = 0;
    virtual void set(const std::string &name, const glm::mat4 &val) const = 0;
};
} // namespace ivx