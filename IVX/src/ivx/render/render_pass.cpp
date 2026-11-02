#include "ivxpch.h"
#include "render_pass.h"

#include "ivx/core/logger.h"

namespace ivx {

RenderPass::RenderPass(RenderPassSpecification spec) : spec_(std::move(spec)) {
}

void RenderPass::begin() {
    IVX_CORE_ASSERT(!begun_, "RenderPass::begin() called twice without end()");
    begun_ = true;

    if (spec_.target) {
        spec_.target->bind(); // framebuffer
    }

    if (has_lights_) {
        light_uniforms_.update(lights_);
    }

    // Re-claims the binding point: another pass may have taken it since the
    // last frame.
    if (spec_.camera) {
        camera_uniforms_.update(*spec_.camera);
    }

    // Snapshot before touching anything: end() has to put back what the
    // *previous* pass left, not a hardcoded default.
    saved_state_ = RenderCommand::state();

    RenderCommand::State wanted{};
    wanted.depth_test = spec_.depth_test;
    wanted.depth_write = spec_.depth_write;
    wanted.depth_func = spec_.depth_func;
    wanted.blend = spec_.blend;
    wanted.cull = spec_.cull;
    wanted.stencil = spec_.stencil;
    RenderCommand::set_state(wanted);

    if (spec_.target) {
        // The framebuffer knows how to clear integer and multisampled
        // attachments; a global glClearColor cannot reach either.
        spec_.target->clear(spec_.clear_color,
                            spec_.clear_color_attachments,
                            spec_.clear_depth);
    } else if (spec_.clear_color_attachments || spec_.clear_depth) {
        RenderCommand::set_clear_color(spec_.clear_color);
        RenderCommand::clear(spec_.clear_color_attachments, spec_.clear_depth);
    }
}

void RenderPass::end() {
    IVX_CORE_ASSERT(begun_, "RenderPass::end() without begin()");
    if (!begun_) {
        return;
    }
    begun_ = false;

    RenderCommand::set_state(saved_state_);

    if (spec_.target) {
        spec_.target->unbind();
    }
}

} // namespace ivx
