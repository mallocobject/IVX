#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/camera.h"
#include "ivx/render/frame_buffer.h"
#include "ivx/scene/scene.h"
#include "ivx/render/render_scope.h"
#include "shadow_uniforms.h"
#include <glm/glm.hpp>

namespace ivx {
class LightSpaceCamera : public Camera {
  public:
    void
    set(glm::mat4 view, glm::mat4 projection, float near_clip, float far_clip) {
        view_ = view;
        projection_ = projection;
        near_ = near_clip;
        far_ = far_clip;
    }
    glm::mat4 get_view_matrix() const override {
        return view_;
    }
    glm::mat4 get_proj_matrix() const override {
        return projection_;
    }
    float get_near() const override {
        return near_;
    }
    float get_far() const override {
        return far_;
    }

  private:
    glm::mat4 view_{1.f}, projection_{1.f};
    float near_{0.1f}, far_{100.f};
};

class ShadowPass : public RefCounted {
  public:
    explicit ShadowPass(uint32_t resolution = 2048);

    bool render(Scene &scene, const LightSet &lights, const Camera &view);
    const GpuShadow &gpu() const {
        return gpu_;
    }
    const Ref<FrameBuffer> &output() const {
        return scope_->target();
    }

  private:
    bool fit_directional(const Light &light, const Camera &view);
    uint32_t resolution_;
    Ref<RenderScope> scope_;
    LightSpaceCamera light_camera_;
    GpuShadow gpu_{};
};

} // namespace ivx
