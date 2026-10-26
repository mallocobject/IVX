#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/renderer/renderer_api.h"
#include <glm/glm.hpp>

namespace ivx {
class OpenGLRendererAPI : public RendererAPI {
  public:
    virtual void init() override;
    virtual void set_viewport(int x, int y, int width, int height) override;
    virtual void set_clear_color(const glm::vec4 &color) override;
    virtual void set_depth_test(bool enabled = true) override;
    virtual void clear() override;
    virtual void draw(const Ref<VertexArray> &vertex_array) override;
    virtual void draw(const Ref<VertexArray> &vertex_array,
                      uint32_t num_indices) override;
};
} // namespace ivx
