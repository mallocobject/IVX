#include "ivxpch.h"
#include "render_scope.h"

#include "ivx/core/logger.h"

namespace ivx {

RenderScope::RenderScope(RenderScopeSpecification spec)
    : spec_(std::move(spec)) {
    if (spec_.target) {
        target_ = FrameBuffer::create(*spec_.target);
    }
}

void RenderScope::begin() {
    IVX_CORE_ASSERT(!begun_, "RenderScope::begin() called twice without end()");
    begun_ = true;

    if (target_) {
        target_->bind();
    }

    if (has_lights_) {
        light_uniforms_.update(lights_);
    }

    // Re-claims the binding point: another scope may have taken it since the
    // last frame.
    if (spec_.camera) {
        camera_uniforms_.update(*spec_.camera);
    }

    // Snapshot before touching anything: end() has to put back what the
    // *previous* scope left, not a hardcoded default.
    saved_state_ = RenderCommand::state();

    RenderCommand::State wanted{};
    wanted.depth_test = spec_.depth_test;
    wanted.depth_write = spec_.depth_write;
    wanted.depth_func = spec_.depth_func;
    wanted.blend = spec_.blend;
    wanted.cull = spec_.cull;
    wanted.stencil = spec_.stencil;
    RenderCommand::set_state(wanted);

    if (target_) {
        // The framebuffer knows how to clear integer and multisampled
        // attachments; a global glClearColor cannot reach either.
        target_->clear(spec_.clear_color,
                       spec_.clear_color_attachments,
                       spec_.clear_depth);
    } else if (spec_.clear_color_attachments || spec_.clear_depth) {
        RenderCommand::set_clear_color(spec_.clear_color);
        RenderCommand::clear(spec_.clear_color_attachments, spec_.clear_depth);
    }
}

void RenderScope::end() {
    IVX_CORE_ASSERT(begun_, "RenderScope::end() without begin()");
    if (!begun_) {
        return;
    }
    begun_ = false;

    RenderCommand::set_state(saved_state_);

    if (target_) {
        target_->unbind();
    }
}

void RenderScope::resize(uint32_t width, uint32_t height) {
    if (!target_ || width == 0 || height == 0) {
        return;
    }

    // Keep the description in step with the live framebuffer, so reading either
    // one gives the same size.
    if (spec_.target) {
        spec_.target->width = width;
        spec_.target->height = height;
    }
    target_->resize(width, height);
}

} // namespace ivx
