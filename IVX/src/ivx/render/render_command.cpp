#include "ivxpch.h"
#include "render_command.h"
#include "ivx/core/ref.hpp"
#include "renderer_api.h"

namespace ivx {
Ref<RendererAPI> RenderCommand::renderer_api_ = RendererAPI::create();
RenderCommand::State RenderCommand::state_{};

void RenderCommand::init() {
    renderer_api_->init();

    // Everything that is not device wide is pushed through the same setters the
    // passes use: the engine default is then simply "what the first pass would
    // have asked for", and the tracked copy matches what the driver really has.
    state_ = State{};
    renderer_api_->set_depth_test(state_.depth_test);
    renderer_api_->set_depth_write(state_.depth_write);
    renderer_api_->set_depth_func(state_.depth_func);
    set_blend_state(state_.blend);
    renderer_api_->set_cull_mode(state_.cull);
    set_stencil_state(state_.stencil);
}

void RenderCommand::set_viewport(int x, int y, int width, int height) {
    renderer_api_->set_viewport(x, y, width, height);
}

void RenderCommand::set_clear_color(const glm::vec4 &color) {
    renderer_api_->set_clear_color(color);
}

void RenderCommand::set_depth_test(bool enabled) {
    state_.depth_test = enabled;
    renderer_api_->set_depth_test(enabled);
}

void RenderCommand::set_depth_write(bool enabled) {
    state_.depth_write = enabled;
    renderer_api_->set_depth_write(enabled);
}

void RenderCommand::set_depth_func(RendererAPI::Cmp func) {
    state_.depth_func = func;
    renderer_api_->set_depth_func(func);
}

void RenderCommand::set_blend(bool enabled) {
    state_.blend.enabled = enabled;
    renderer_api_->set_blend(enabled);
}

void RenderCommand::set_blend_func(RendererAPI::BlendFactor src,
                                   RendererAPI::BlendFactor dst) {
    state_.blend.src = src;
    state_.blend.dst = dst;
    renderer_api_->set_blend_func(src, dst);
}

void RenderCommand::set_blend_equation(RendererAPI::BlendEquation equation) {
    state_.blend.equation = equation;
    renderer_api_->set_blend_equation(equation);
}

void RenderCommand::set_blend_state(const BlendState &blend) {
    set_blend(blend.enabled);
    set_blend_func(blend.src, blend.dst);
    set_blend_equation(blend.equation);
}

void RenderCommand::set_cull_mode(RendererAPI::CullMode mode) {
    state_.cull = mode;
    renderer_api_->set_cull_mode(mode);
}

void RenderCommand::set_stencil_test(bool enabled) {
    state_.stencil.enabled = enabled;
    renderer_api_->set_stencil_test(enabled);
}

void RenderCommand::set_stencil_func(RendererAPI::Cmp func,
                                     int ref,
                                     uint32_t read_mask) {
    state_.stencil.func = func;
    state_.stencil.ref = ref;
    state_.stencil.read_mask = read_mask;
    renderer_api_->set_stencil_func(func, ref, read_mask);
}

void RenderCommand::set_stencil_op(RendererAPI::StencilOp fail,
                                   RendererAPI::StencilOp depth_fail,
                                   RendererAPI::StencilOp pass) {
    state_.stencil.fail = fail;
    state_.stencil.depth_fail = depth_fail;
    state_.stencil.pass = pass;
    renderer_api_->set_stencil_op(fail, depth_fail, pass);
}

void RenderCommand::set_stencil_write_mask(uint32_t mask) {
    state_.stencil.write_mask = mask;
    renderer_api_->set_stencil_write_mask(mask);
}

void RenderCommand::set_stencil_state(const StencilState &stencil) {
    set_stencil_test(stencil.enabled);
    set_stencil_func(stencil.func, stencil.ref, stencil.read_mask);
    set_stencil_op(stencil.fail, stencil.depth_fail, stencil.pass);
    set_stencil_write_mask(stencil.write_mask);
}

void RenderCommand::set_state(const State &state) {
    if (state_.depth_test != state.depth_test) {
        set_depth_test(state.depth_test);
    }
    if (state_.depth_write != state.depth_write) {
        set_depth_write(state.depth_write);
    }
    if (state_.depth_func != state.depth_func) {
        set_depth_func(state.depth_func);
    }
    if (state_.blend != state.blend) {
        set_blend_state(state.blend);
    }
    if (state_.cull != state.cull) {
        set_cull_mode(state.cull);
    }
    if (state_.stencil != state.stencil) {
        set_stencil_state(state.stencil);
    }
}

void RenderCommand::clear(bool clear_color, bool clear_depth) {
    renderer_api_->clear(clear_color, clear_depth);
}

void RenderCommand::draw(const Ref<VertexArray> &vertex_array) {
    renderer_api_->draw(vertex_array);
}

void RenderCommand::draw(const Ref<VertexArray> &vertex_array,
                         uint32_t num_indices) {
    renderer_api_->draw(vertex_array, num_indices);
}

} // namespace ivx
