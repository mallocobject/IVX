#pragma once

#include "ivx/render/light_uniforms.h"
#include "ivx/render/shadow_pass.h"
#include "ivx/render/tone_mapping_pass.h"

namespace ivx {
class SceneRenderer : public RefCounted {
  public:
    SceneRenderer(uint32_t width, uint32_t height);
    void resize(uint32_t width, uint32_t height);
    void render(Scene *scene, const Camera &camera);
    uint32_t read_entity(uint32_t x, uint32_t y) const;
    const Ref<FrameBuffer> &output() const {
        return tone_mapping_.output();
    }
    const Ref<FrameBuffer> &hdr_target() const {
        return scene_scope_.target();
    }

  private:
    RenderScope scene_scope_;
    ShadowPass shadows_;
    ToneMappingPass tone_mapping_;
    LightUniforms light_uniforms_;
    ShadowUniforms shadow_uniforms_;
};
} // namespace ivx
