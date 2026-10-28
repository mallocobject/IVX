#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

namespace ivx {
class RendererAPI : public RefCounted {
  public:
    enum class API : uint8_t {
        kNone = 0,
        kOpenGL,
        kDirectX,
        kVulkan,
        kMetal,
    };

    virtual void init() = 0;
    virtual void set_viewport(int x, int y, int width, int height) = 0;
    virtual void set_clear_color(const glm::vec4 &color) = 0;
    virtual void set_depth_test(bool enabled = true) = 0;
    virtual void clear() = 0;
    virtual void draw(const Ref<VertexArray> &vertex_array) = 0;
    virtual void draw(const Ref<VertexArray> &vertex_array,
                      uint32_t num_indices) = 0;

    static API get_api() {
        return api_;
    }

    static Ref<RendererAPI> create();

  private:
    static API api_;
};
} // namespace ivx
