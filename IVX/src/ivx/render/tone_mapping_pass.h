#pragma once

#include "ivx/render/render_scope.h"
#include "ivx/render/shader.h"
#include "ivx/render/tone_mapping_curve.h"
#include "ivx/render/vertex_array.h"

namespace ivx {

class ToneMappingPass : public RefCounted {
  public:
    ToneMappingPass(uint32_t width, uint32_t height);
    void resize(uint32_t width, uint32_t height);
    void render(const Ref<FrameBuffer> &source,
                float exposure,
                ToneMappingCurve curve);
    const Ref<FrameBuffer> &output() const {
        return scope_->target();
    }

  private:
    Ref<RenderScope> scope_;
    Ref<Shader> shader_;
    Ref<VertexArray> triangle_;
};
} // namespace ivx
