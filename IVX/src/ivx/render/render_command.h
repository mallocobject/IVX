#pragma once

#include "ivx/core/ref.hpp"
#include "renderer_api.h"
#include "vertex_array.h"
#include <glm/glm.hpp>

namespace ivx {
class RenderCommand {
  public:
    static void init();
    static void set_clear_color(const glm::vec4 &color);
    static void clear();
    static void draw(const Ref<VertexArray> &vertex_array);

  private:
    static RendererAPI *renderer_api_;
};
} // namespace ivx