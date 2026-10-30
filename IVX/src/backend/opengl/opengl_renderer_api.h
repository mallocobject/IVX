#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/renderer_api.h"
#include <glm/glm.hpp>

namespace ivx {
class OpenGLRendererAPI : public RendererAPI {
  public:
    virtual void init() override;
    virtual void set_viewport(int x, int y, int width, int height) override;
    virtual void set_clear_color(const glm::vec4 &color) override;
    virtual void set_depth_test(bool enabled = true) override;
    virtual void set_depth_write(bool enabled = true) override;
    virtual void set_blend(bool enabled = true) override;
    virtual void set_cull_faces(bool enabled = true) override;
    virtual void clear(bool clear_color = true,
                       bool clear_depth = true) override;
    virtual void draw(const Ref<VertexArray> &vertex_array) override;
    virtual void draw(const Ref<VertexArray> &vertex_array,
                      uint32_t num_indices) override;
};
} // namespace ivx
