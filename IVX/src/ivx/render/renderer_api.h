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
    // Depth writes are separate from the depth test: a depth-only prepass or a
    // 2D overlay pass needs one without the other.
    virtual void set_depth_write(bool enabled = true) = 0;
    virtual void set_blend(bool enabled = true) = 0;
    virtual void set_cull_faces(bool enabled = true) = 0;
    // The two halves are independent so a depth-only pass can skip the colour
    // clear.
    virtual void clear(bool clear_color = true, bool clear_depth = true) = 0;
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
