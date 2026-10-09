#pragma once

#include "renderer_api.h"
#include "vertex_array.h"
#include <glm/glm.hpp>

namespace ivx {
class RenderCommand {
  public:
    static void set_clear_color(const glm::vec4 &color);
    static void clear();
    static void draw(const std::shared_ptr<VertexArray> &vertex_array);

  private:
    static RendererAPI *renderer_api_;
};
} // namespace ivx