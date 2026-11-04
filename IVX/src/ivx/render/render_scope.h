#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include "ivx/render/frame_buffer.h"
#include "ivx/render/render_command.h"
#include <glm/glm.hpp>
#include <optional>

namespace ivx {

struct RenderScopeSpecification {
    // No target means the window framebuffer.
    std::optional<FrameBufferSpecification> target;

    glm::vec4 clear_color{0.1f, 0.1f, 0.1f, 1.f};
    bool clear_color_attachments{true};
    bool clear_depth{true};

    bool depth_test{true};
    bool depth_write{true};
    RendererAPI::Cmp depth_func{RendererAPI::Cmp::IVX_Greater};
    BlendState blend{};
    RendererAPI::CullMode cull{RendererAPI::CullMode::IVX_None};
    StencilState stencil{};
};

class RenderScope : public RefCounted {
  public:
    explicit RenderScope(RenderScopeSpecification spec);
    ~RenderScope() = default;

    void begin();
    void end();

    const Ref<FrameBuffer> &target() const {
        return target_;
    }
    void resize(uint32_t width, uint32_t height);

    const RenderScopeSpecification &get_specification() const {
        return spec_;
    }

  private:
    RenderScopeSpecification spec_;
    Ref<FrameBuffer> target_;
    RenderCommand::State saved_state_{};
    bool begun_{false};
};

} // namespace ivx
