#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/renderer_api.h"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {
class RenderCommand {
  public:
    static void init();
    static void set_viewport(int x, int y, int width, int height);
    static void set_clear_color(const glm::vec4 &color);
    static void set_depth_test(bool enabled = true);
    static void clear();
    static void draw(const Ref<VertexArray> &vertex_array);
    static void draw(const Ref<VertexArray> &vertex_array,
                     uint32_t num_indices);

  private:
    static Ref<RendererAPI> renderer_api_;
};
} // namespace ivx
