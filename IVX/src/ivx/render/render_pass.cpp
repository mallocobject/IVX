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
        spec_.target->bind();
    }

    // Snapshot before touching anything: end() has to put back what the
    // *previous* pass left, not a hardcoded default.
    saved_state_ = RenderCommand::state();
    RenderCommand::set_depth_test(spec_.depth_test);
    RenderCommand::set_depth_write(spec_.depth_write);
    RenderCommand::set_blend(spec_.blend);
    RenderCommand::set_cull_faces(spec_.cull_faces);

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

    RenderCommand::set_depth_test(saved_state_.depth_test);
    RenderCommand::set_depth_write(saved_state_.depth_write);
    RenderCommand::set_blend(saved_state_.blend);
    RenderCommand::set_cull_faces(saved_state_.cull_faces);

    if (spec_.target) {
        spec_.target->unbind();
    }
}

} // namespace ivx
