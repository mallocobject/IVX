#pragma once

#include <cstdint>
#include <glad/glad.h>
#include <string>

namespace ivx {
class Shader {
  public:
    // Shader(const std::string &vs_src, const std::string &fs_src);
    virtual ~Shader() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;

    static Shader *create(const std::string &vs_src, const std::string &fs_src);
};
} // namespace ivx