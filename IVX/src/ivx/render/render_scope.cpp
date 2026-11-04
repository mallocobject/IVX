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

    if (spec_.target) {
        spec_.target->width = width;
        spec_.target->height = height;
    }
    target_->resize(width, height);
}

} // namespace ivx
