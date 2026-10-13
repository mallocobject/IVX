#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <cstdint>
#include <string>

#include <glm/glm.hpp>

namespace ivx {
class Shader : public RefCounted {
  public:
    // Shader(const std::string &vs_src, const std::string &fs_src);
    virtual ~Shader() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    static Ref<Shader> create(const std::string &vs_src,
                              const std::string &fs_src);
    static Ref<Shader> create(const std::string &path);

    virtual void set(const std::string &name, bool val) = 0;
    virtual void set(const std::string &name, float val) = 0;
    virtual void set(const std::string &name, int val) = 0;

    virtual void set(const std::string &name, const glm::vec2 &val) = 0;
    virtual void set(const std::string &name, const glm::vec3 &val) = 0;
    virtual void set(const std::string &name, const glm::vec4 &val) = 0;

    virtual void set(const std::string &name, const glm::ivec2 &val) = 0;
    virtual void set(const std::string &name, const glm::ivec3 &val) = 0;
    virtual void set(const std::string &name, const glm::ivec4 &val) = 0;

    virtual void set(const std::string &name, const glm::mat3 &val) = 0;
    virtual void set(const std::string &name, const glm::mat4 &val) = 0;
};
} // namespace ivx
