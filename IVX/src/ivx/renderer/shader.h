#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <cstdint>
#include <filesystem>
#include <string>

#include <glm/glm.hpp>
#include <unordered_map>

namespace ivx {
class Shader : public RefCounted {
  public:
    // Shader(const std::string &vs_src, const std::string &fs_src);
    virtual ~Shader() = default;

    virtual void bind() = 0;
    virtual void unbind() = 0;
    virtual const std::string &get_label() const = 0;

    static Ref<Shader> create(const std::string &vs_src,
                              const std::string &fs_src,
                              const std::string &label);
    static Ref<Shader> create(const std::filesystem::path &filepath,
                              const std::string &label = {});

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

class ShaderLibrary {
  public:
    static void add(const Ref<Shader> &shader);
    static Ref<Shader> load(const std::filesystem::path &filepath,
                            const std::string &label = {});
    static Ref<Shader> get(const std::string &label);

  private:
    inline static std::unordered_map<std::string, Ref<Shader>> shaders_;
};
} // namespace ivx
