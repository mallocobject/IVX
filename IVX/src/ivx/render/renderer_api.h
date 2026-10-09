#pragma once

#include "vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
class RendererAPI {
  public:
    enum class API : uint8_t {
        kNone = 0,
        kOpenGL,
        kDirectX,
        kVulkan,
        kMetal,
    };

    virtual void set_clear_color(const glm::vec4 &color) = 0;
    virtual void clear() = 0;
    virtual void draw(const std::shared_ptr<VertexArray> &vertex_array) = 0;

    static API get_api() {
        return api_;
    }

  private:
    static API api_;
};
} // namespace ivx