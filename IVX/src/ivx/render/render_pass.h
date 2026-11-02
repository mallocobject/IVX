#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/camera_uniforms.h"
#include "ivx/render/frame_buffer.h"
#include "ivx/render/light.h"
#include "ivx/render/light_uniforms.h"
#include "ivx/render/render_command.h"
#include <glm/glm.hpp>

namespace ivx {

class Camera;

// One pass: a target, the camera, what to clear, and the pipeline state the
// pass needs. Everything a pass changes is put back by end().
struct RenderPassSpecification {
    // nullptr draws to the default framebuffer (the window).
    Ref<FrameBuffer> target;
    // nullptr leaves whatever camera block is currently bound in place.
    const Camera *camera{nullptr};

    glm::vec4 clear_color{0.1f, 0.1f, 0.1f, 1.f};
    bool clear_color_attachments{true};
    bool clear_depth{true};

    bool depth_test{true};
    bool depth_write{true};
    bool blend{true};
    // Off by default: sprite quads are single sided.
    bool cull_faces{false};
};

class RenderPass : public RefCounted {
  public:
    explicit RenderPass(RenderPassSpecification spec);
    ~RenderPass() = default;

    void begin();
    void end();

    const RenderPassSpecification &get_specification() const {
        return spec_;
    }
    // The editor swaps its framebuffer when the viewport is resized.
    void set_target(Ref<FrameBuffer> target) {
        spec_.target = std::move(target);
    }
    void set_camera(const Camera *camera) {
        spec_.camera = camera;
    }
    // Gathered by the scene every frame, so the pass keeps its own copy.
    void set_lights(const LightSet &lights) {
        lights_ = lights;
        has_lights_ = true;
    }

  private:
    RenderPassSpecification spec_;
    // Per pass camera block, so two passes with different cameras can coexist.
    CameraUniforms camera_uniforms_;
    LightSet lights_{};
    LightUniforms light_uniforms_;
    bool has_lights_{false};
    RenderCommand::State saved_state_{};
    bool begun_{false};
};

} // namespace ivx
