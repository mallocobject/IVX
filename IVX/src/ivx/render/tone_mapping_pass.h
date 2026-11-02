#pragma once

#include "ivx/render/render_scope.h"
#include "ivx/render/shader.h"
#include "ivx/render/tone_mapping_curve.h"
#include "ivx/render/vertex_array.h"

namespace ivx {

// Converts resolved linear HDR colour to a display-ready RGBA8 target. Entity
// IDs remain in the source framebuffer and bypass this pass.
//
// It owns its render scope (and therefore its output target), the opposite of
// Scene::render(): tone mapping is one self-contained full-screen draw whose
// target and state are nobody else's business. Anything that has to be
// sequenced with other passes (shadow, picking, a bloom chain) belongs to the
// caller instead, the way EditLayer drives the scene scope.
class ToneMappingPass : public RefCounted {
  public:
    ToneMappingPass(uint32_t width, uint32_t height);
    void resize(uint32_t width, uint32_t height);
    void render(const Ref<FrameBuffer> &source,
                float exposure,
                ToneMappingCurve curve);
    const Ref<FrameBuffer> output() const {
        return scope_->target();
    }

  private:
    Ref<RenderScope> scope_;
    Ref<Shader> shader_;
    Ref<VertexArray> triangle_;
};
} // namespace ivx
