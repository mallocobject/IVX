#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/frame_buffer.h"
#include "ivx/render/render_command.h"
#include <glm/glm.hpp>

namespace ivx {

// One pass: a target, what to clear, and the pipeline state the pass needs.
// Everything a pass changes is put back by end(), so passes can be chained in
// any order without leaking state into each other - the reason this exists at
// all is that the editor used to bind / clear / unbind its framebuffer by hand
// while the 2D renderer set its own state in begin_scene().
struct RenderPassSpecification {
    // nullptr draws to the default framebuffer (the window).
    Ref<FrameBuffer> target;
    glm::vec4 clear_color{0.1f, 0.1f, 0.1f, 1.f};
    bool clear_color_attachments{true};
    bool clear_depth{true};

    bool depth_test{true};
    bool depth_write{true};
    bool blend{true};
    // Off by default: sprite quads are single sided and the 2D path relies on
    // seeing both sides. A 3D pass turns it on.
    bool cull_faces{false};
};

class RenderPass : public RefCounted {
  public:
    explicit RenderPass(RenderPassSpecification spec);
    ~RenderPass() = default;

    // Binds the target, applies the state, clears. Pairs with end().
    void begin();
    // Puts the state back and unbinds the target (which also resolves a
    // multisampled framebuffer).
    void end();

    const RenderPassSpecification &get_specification() const {
        return spec_;
    }
    // The editor swaps its framebuffer when the viewport is resized; the pass
    // keeps describing the same pass, only the attachment object changes.
    void set_target(Ref<FrameBuffer> target) {
        spec_.target = std::move(target);
    }

  private:
    RenderPassSpecification spec_;
    RenderCommand::State saved_state_{};
    bool begun_{false};
};

} // namespace ivx
