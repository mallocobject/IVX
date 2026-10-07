#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
class Renderer {
  public:
    enum class API : uint8_t {
        kNone = 0,
        kOpenGL,
        kDirectX,
        kVulkan,
        kMetal,
    };

    virtual ~Renderer() {
    }

    virtual void set_clear_color(const glm::vec4 &color) = 0;
    virtual void clear() = 0;
    virtual void draw() = 0;

    static API get_api() {
        return api_;
    }

  private:
    static API api_;
};
} // namespace ivx