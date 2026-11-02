#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/renderer_api.h"
#include "ivx/render/vertex_array.h"
#include <cstdint>
#include <glm/glm.hpp>

namespace ivx {

// The stencil half of the pipeline state, as a pass says it wants it.
struct StencilState {
    bool enabled{false};
    RendererAPI::Cmp func{RendererAPI::Cmp::IVX_Always};
    int ref{0};
    uint32_t read_mask{0xFFu};
    RendererAPI::StencilOp fail{RendererAPI::StencilOp::IVX_Keep};
    RendererAPI::StencilOp depth_fail{RendererAPI::StencilOp::IVX_Keep};
    RendererAPI::StencilOp pass{RendererAPI::StencilOp::IVX_Keep};
    uint32_t write_mask{0xFFu};

    bool operator==(const StencilState &) const = default;
};

// The blending half: whether to blend, the weights, and the equation itself.
struct BlendState {
    bool enabled{true};
    RendererAPI::BlendFactor src{RendererAPI::BlendFactor::IVX_SrcAlpha};
    RendererAPI::BlendFactor dst{
        RendererAPI::BlendFactor::IVX_OneMinusSrcAlpha};
    RendererAPI::BlendEquation equation{RendererAPI::BlendEquation::IVX_Add};

    bool operator==(const BlendState &) const = default;
};

class RenderCommand {
  public:
    // The state a scope can change. It is tracked here so RenderScope can put
    // back what it found instead of guessing a default: with several passes per
    // frame, the previous values are the only correct ones to restore.
    struct State {
        bool depth_test{true};
        bool depth_write{true};
        RendererAPI::Cmp depth_func{RendererAPI::Cmp::IVX_Greater}; // reverse-Z
        BlendState blend{};
        RendererAPI::CullMode cull{RendererAPI::CullMode::IVX_Back};
        StencilState stencil{};

        bool operator==(const State &) const = default;
    };

    // The device itself starts with none of this on (see RendererAPI::init), so
    // init() pushes State{} through the same setters a pass uses. That leaves
    // the tracked copy truthful before the first pass diffs against it.
    static void init();

    static void set_viewport(int x, int y, int width, int height);
    static void set_clear_color(const glm::vec4 &color);

    static void set_depth_test(bool enabled = true);
    static void set_depth_write(bool enabled = true);
    static void
    set_depth_func(RendererAPI::Cmp func = RendererAPI::Cmp::IVX_Greater);

    static void set_blend(bool enabled = true);
    static void set_blend_func(
        RendererAPI::BlendFactor src = RendererAPI::BlendFactor::IVX_SrcAlpha,
        RendererAPI::BlendFactor dst =
            RendererAPI::BlendFactor::IVX_OneMinusSrcAlpha);
    static void set_blend_equation(RendererAPI::BlendEquation equation =
                                       RendererAPI::BlendEquation::IVX_Add);
    static void set_blend_state(const BlendState &blend);

    static void
    set_cull_mode(RendererAPI::CullMode mode = RendererAPI::CullMode::IVX_Back);

    static void set_stencil_test(bool enabled = true);
    // The read mask filters the comparison; the write mask is separate because
    // the usual pattern is "test against 0xFF, write only 0x01".
    static void
    set_stencil_func(RendererAPI::Cmp func = RendererAPI::Cmp::IVX_Always,
                     int ref = 0,
                     uint32_t read_mask = 0xFF);
    // fail / depth-fail / depth-pass, in that order, like glStencilOp.
    static void set_stencil_op(
        RendererAPI::StencilOp fail = RendererAPI::StencilOp::IVX_Keep,
        RendererAPI::StencilOp depth_fail = RendererAPI::StencilOp::IVX_Keep,
        RendererAPI::StencilOp pass = RendererAPI::StencilOp::IVX_Keep);
    static void set_stencil_write_mask(uint32_t mask = 0xFF);
    static void set_stencil_state(const StencilState &stencil);

    // Applies only the fields that differ from the tracked state, so
    // consecutive passes that agree cost no driver call for those fields.
    static void set_state(const State &state);
    static const State &state() {
        return state_;
    }
    static void clear(bool clear_color = true, bool clear_depth = true);
    static void draw(const Ref<VertexArray> &vertex_array);
    static void draw(const Ref<VertexArray> &vertex_array,
                     uint32_t num_indices);

  private:
    static Ref<RendererAPI> renderer_api_;
    static State state_;
};
} // namespace ivx
