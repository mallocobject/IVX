#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/camera.h"
#include "ivx/render/frame_buffer.h"
#include "ivx/render/render_scope.h"
#include "ivx/scene/scene.h"
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
    const GpuShadowData &gpu() const {
        return gpu_;
    }
    const Ref<FrameBuffer> &output(uint32_t light_index) const {
        return scopes_[light_index] ? scopes_[light_index]->target()
                                    : empty_map_;
    }
    void bind_maps() const;

  private:
    bool fit_directional(const Light &light,
                         const Camera &view,
                         float distance,
                         GpuShadow &gpu);
    uint32_t resolution_;
    std::array<Ref<RenderScope>, k_max_lights> scopes_{};
    Ref<FrameBuffer> empty_map_;
    LightSpaceCamera light_camera_;
    GpuShadowData gpu_{};
};

} // namespace ivx
