#pragma once

#include "glm/ext/vector_float4.hpp"
#include "ivx/render/frame_buffer.h"
#include <cstdint>
#include <vector>

namespace ivx {
class OpenGLFrameBuffer : public FrameBuffer {
  public:
    OpenGLFrameBuffer(const FrameBufferSpecification &spec);
    virtual ~OpenGLFrameBuffer();

    // With multisampling on this is the *resolved* texture: the multisampled
    // buffers cannot be sampled directly, so unbind() blits them into
    // single-sampled textures that ImGui and the shaders can read.
    virtual uint32_t get_color_attachment(uint32_t index = 0) const override;
    virtual void bind_color_attachment(uint32_t slot,
                                       uint32_t index = 0) const override;

    virtual uint32_t get_depth_attachment() const override;
    virtual void bind_depth_attachment(uint32_t slot) const override;
    void set_depth_face(uint32_t face) override;

    // Reads the last resolved frame (see unbind()). Coordinates start
    // bottom-left.
    virtual uint32_t
    read_pixel(uint32_t index, uint32_t x, uint32_t y) const override;

    virtual void clear(const glm::vec4 &clear_color,
                       bool clear_color_attachments = true,
                       bool clear_depth = true) override;

    virtual const FrameBufferSpecification &get_specification() const override {
        return spec_;
    }

    virtual void bind() override;
    // Resolves the multisampled attachments (a blit) and restores the
    // previously bound framebuffer and viewport.
    virtual void unbind() override;

    virtual void resize(uint32_t width, uint32_t height) override;

  private:
    // One colour attachment: the sampleable texture (also the resolve target)
    // and, with multisampling on, the renderbuffer that is actually drawn into.
    struct ColorAttachment {
        FrameBufferColorFormat format{FrameBufferColorFormat::RGBA8};
        uint32_t texture{0};
        uint32_t renderbuffer{0};
    };

    // Either a sampleable texture or a renderbuffer, never both.
    struct DepthAttachment {
        FrameBufferDepthFormat format{FrameBufferDepthFormat::None};
        uint32_t texture{0};
        uint32_t renderbuffer{0};
    };

    void invalidate();
    // Releases the GL objects only; invalidate() then builds a fresh set. Kept
    // separate from the destructor so resizing does not have to duplicate the
    // deletion list.
    void destroy();

    // The single sampled storage: the textures a shader samples (and the
    // resolve targets of a multisampled framebuffer).
    void create_color_targets();
    // `samples` is 1 for the single sampled case and > 1 for the draw target.
    void create_depth_target(int samples);

    void invalidate_multisampled();
    void resolve() const;

    uint32_t fbo_{0};
    std::vector<ColorAttachment> color_attachments_;
    DepthAttachment depth_;

    // Multisampled mode only: the FBO the single-sampled textures resolve into.
    uint32_t resolve_fbo_{0};

    FrameBufferSpecification spec_;

    int previous_fbo_{0};
    int previous_viewport_[4]{0};
};
} // namespace ivx
