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
        IVX_None = 0,
        IVX_OpenGL,
        IVX_DirectX,
        IVX_Vulkan,
        IVX_Metal,
    };

    // Shared by the depth test and the stencil test. Mind the reverse-Z default
    // of this engine: a closer fragment has a greater depth, so depth testing
    // uses IVX_Greater.
    enum class Cmp : uint8_t {
        IVX_Never = 0,
        IVX_Less,
        IVX_Equal,
        IVX_Lequal,
        IVX_Greater,
        IVX_Nequal,
        IVX_Gequal,
        IVX_Always,
    };

    enum class StencilOp : uint8_t {
        IVX_Keep = 0,
        IVX_Zero,
        IVX_Replace,
        IVX_IncrClamp,
        IVX_DecrClamp,
        IVX_Invert,
        IVX_IncrWrap,
        IVX_DecrWrap,
    };

    // Source and destination weights of the blend equation.
    enum class BlendFactor : uint8_t {
        IVX_Zero = 0,
        IVX_One,
        IVX_SrcAlpha,
        IVX_OneMinusSrcAlpha,
        IVX_DstAlpha,
        IVX_OneMinusDstAlpha,
        IVX_SrcColor,
        IVX_OneMinusSrcColor,
        IVX_DstColor,
        IVX_OneMinusDstColor,
    };

    enum class BlendEquation : uint8_t {
        IVX_Add = 0,
        IVX_Subtract,
        IVX_ReverseSubtract,
        IVX_Min,
        IVX_Max,
    };

    // None disables face culling; the others pick which side is dropped.
    // Culling the front is how a shadow pass trades peter-panning for acne.
    enum class CullMode : uint8_t {
        IVX_None = 0,
        IVX_Front,
        IVX_Back,
    };

    // Device wide setup only: debug output, the winding order the meshes are
    // authored with, and the clip convention the projection helpers assume.
    // Anything a single pass could want to change lives in
    // RenderScopeSpecification instead.
    virtual void init() = 0;
    virtual uint32_t max_fragment_texture_units() const = 0;

    virtual void set_viewport(int x, int y, int width, int height) = 0;
    virtual void set_clear_color(const glm::vec4 &color) = 0;
    virtual void set_depth_test(bool enabled = true) = 0;
    // Depth writes are separate from the depth test: a depth-only prepass or a
    // 2D overlay pass needs one without the other.
    virtual void set_depth_write(bool enabled = true) = 0;
    virtual void set_depth_func(Cmp func = Cmp::IVX_Greater) = 0;
    virtual void set_blend(bool enabled = true) = 0;
    virtual void
    set_blend_func(BlendFactor src = BlendFactor::IVX_SrcAlpha,
                   BlendFactor dst = BlendFactor::IVX_OneMinusSrcAlpha) = 0;
    virtual void
    set_blend_equation(BlendEquation equation = BlendEquation::IVX_Add) = 0;
    virtual void set_cull_mode(CullMode mode = CullMode::IVX_Back) = 0;

    virtual void set_stencil_test(bool enabled = true) = 0;
    // The read mask filters the comparison; the write mask is separate because
    // the usual pattern is "test against 0xFF, write only 0x01".
    virtual void set_stencil_func(Cmp func = Cmp::IVX_Always,
                                  int ref = 0,
                                  uint32_t read_mask = 0xFF) = 0;
    // fail / depth-fail / depth-pass, in that order, like glStencilOp.
    virtual void set_stencil_op(StencilOp fail = StencilOp::IVX_Keep,
                                StencilOp depth_fail = StencilOp::IVX_Keep,
                                StencilOp pass = StencilOp::IVX_Keep) = 0;
    virtual void set_stencil_write_mask(uint32_t mask = 0xFF) = 0;

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
