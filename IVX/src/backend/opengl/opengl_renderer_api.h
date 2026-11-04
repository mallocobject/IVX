#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/renderer_api.h"
#include <glm/glm.hpp>

namespace ivx {
class OpenGLRendererAPI : public RendererAPI {
  public:
    virtual void init() override;
    uint32_t max_fragment_texture_units() const override;
    virtual void set_viewport(int x, int y, int width, int height) override;
    virtual void set_clear_color(const glm::vec4 &color) override;
    virtual void set_depth_test(bool enabled = true) override;
    virtual void set_depth_write(bool enabled = true) override;
    virtual void set_depth_func(Cmp func = Cmp::IVX_Greater) override;
    virtual void set_blend(bool enabled = true) override;
    virtual void set_blend_func(
        BlendFactor src = BlendFactor::IVX_SrcAlpha,
        BlendFactor dst = BlendFactor::IVX_OneMinusSrcAlpha) override;
    virtual void set_blend_equation(
        BlendEquation equation = BlendEquation::IVX_Add) override;
    virtual void set_cull_mode(CullMode mode = CullMode::IVX_Back) override;

    virtual void set_stencil_test(bool enabled = true) override;
    virtual void set_stencil_func(Cmp func = Cmp::IVX_Always,
                                  int ref = 0,
                                  uint32_t read_mask = 0xFF) override;
    virtual void set_stencil_op(StencilOp fail = StencilOp::IVX_Keep,
                                StencilOp depth_fail = StencilOp::IVX_Keep,
                                StencilOp pass = StencilOp::IVX_Keep) override;
    virtual void set_stencil_write_mask(uint32_t mask = 0xFF) override;

    virtual void clear(bool clear_color = true,
                       bool clear_depth = true) override;
    virtual void draw(const Ref<VertexArray> &vertex_array) override;
    virtual void draw(const Ref<VertexArray> &vertex_array,
                      uint32_t num_indices) override;
};
} // namespace ivx
