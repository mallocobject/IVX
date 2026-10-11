#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/renderer_api.h"
#include <glm/glm.hpp>

namespace ivx {
class OpenGLRendererAPI : public RendererAPI {
  public:
    virtual void set_clear_color(const glm::vec4 &color) override;
    virtual void clear() override;
    virtual void draw(const Ref<VertexArray> &vertex_array) override;
};
} // namespace ivx