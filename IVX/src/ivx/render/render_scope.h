#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/camera_uniforms.h"
#include "ivx/render/frame_buffer.h"
#include "ivx/render/light.h"
#include "ivx/render/light_uniforms.h"
#include "ivx/render/render_command.h"
#include <glm/glm.hpp>
#include <optional>

namespace ivx {

class Camera;

// The environment of one stretch of drawing: which target, which camera, what
// to clear, and the pipeline state the draws need. Everything it changes is put
// back by end().
//
// It owns its target: an empty `target` means the default framebuffer (the
// window), anything else is created once from the description and handed out by
// target(). That keeps the whole scope - target included - in one declaration.
struct RenderScopeSpecification {
    std::optional<FrameBufferSpecification> target;
    // nullptr leaves whatever camera block is currently bound in place.
    const Camera *camera{nullptr};

    glm::vec4 clear_color{0.1f, 0.1f, 0.1f, 1.f};
    bool clear_color_attachments{true};
    bool clear_depth{true};

    bool depth_test{true};
    bool depth_write{true};
    // Reverse-Z by default, matching what RenderCommand::init() pushes.
    RendererAPI::Cmp depth_func{RendererAPI::Cmp::IVX_Greater};
    BlendState blend{};
    // Off by default: sprite quads are single sided.
    RendererAPI::CullMode cull{RendererAPI::CullMode::IVX_None};
    // Off by default; a scope that wants masking or outlines asks for it here.
    StencilState stencil{};
};

class RenderScope : public RefCounted {
  public:
    explicit RenderScope(RenderScopeSpecification spec);
    ~RenderScope() = default;

    void begin();
    void end();

    // Null when this scope draws to the default framebuffer.
    const Ref<FrameBuffer> &target() const {
        return target_;
    }
    // Resizes the owned target; a zero size is ignored.
    void resize(uint32_t width, uint32_t height);

    const RenderScopeSpecification &get_specification() const {
        return spec_;
    }
    // Gathered by the scene every frame, so the scope keeps its own copy.
    void set_lights(const LightSet &lights) {
        lights_ = lights;
        has_lights_ = true;
    }

  private:
    RenderScopeSpecification spec_;
    Ref<FrameBuffer> target_;
    // Per scope camera block, so two scopes with different cameras can coexist.
    CameraUniforms camera_uniforms_;
    LightSet lights_{};
    LightUniforms light_uniforms_;
    bool has_lights_{false};
    RenderCommand::State saved_state_{};
    bool begun_{false};
};

} // namespace ivx
