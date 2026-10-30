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

    // Reads the last resolved frame (see unbind()). Coordinates start
    // bottom-left.
    virtual int
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
    void invalidate();
    // Releases the GL objects only; invalidate() then builds a fresh set. Kept
    // separate from the destructor so resizing does not have to duplicate the
    // deletion list.
    void destroy();

    // Multisampled targets only: the FBO that is drawn into, the single-sampled
    // textures that unbind() resolves to, and the multisampled colour
    // renderbuffers.
    void invalidate_multisampled();
    void resolve() const;

    uint32_t fbo_{0};
    // One texture per entry in spec_.attachments; index i is
    // GL_COLOR_ATTACHMENT<i>. Holds the single-sampled textures in both modes:
    // without multisampling they are attached to fbo_ directly, with it they
    // live on resolve_fbo_.
    std::vector<uint32_t> color_attachments_;
    std::vector<FrameBufferTextureFormat> color_formats_;

    uint32_t depth_buffer_{0};

    // Multisampled mode: color attachments as renderbuffers and the FBO they
    // resolve into.
    uint32_t resolve_fbo_{0};
    std::vector<uint32_t> multisampled_attachments_;

    FrameBufferSpecification spec_;

    int previous_fbo_{0};
    int previous_viewport_[4]{0};
};
} // namespace ivx
