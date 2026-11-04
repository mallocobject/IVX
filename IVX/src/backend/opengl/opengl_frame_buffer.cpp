#include "ivxpch.h"
#include "opengl_frame_buffer.h"

#include "ivx/core/logger.h"
#include <cstdint>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace ivx {
namespace {
// Everything that differs between colour formats lives here, so the attachment
// code never switches on a format again: adding a format is one row.
struct GlColorFormat {
    GLenum internal_format;
    bool integer; // no filtering, and the clear has to use the uiv entry point
};

GlColorFormat color_format_of(FrameBufferColorFormat format) {
    using enum FrameBufferColorFormat;
    switch (format) {
    case RGBA16F:
        return {GL_RGBA16F, false};
    case RED_INTEGER:
        return {GL_R32UI, true};
    case RGBA8:
    default:
        return {GL_RGBA8, false};
    }
}

uint32_t create_color_texture(FrameBufferColorFormat format,
                              uint32_t width,
                              uint32_t height) {
    const GlColorFormat info = color_format_of(format);

    uint32_t texture = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, info.internal_format, width, height);
    // Integer targets must not be filtered, or read_pixel() gets interpolated
    // ids.
    const GLint filter = info.integer ? GL_NEAREST : GL_LINEAR;
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, filter);
    return texture;
}

// An empty list has to be spelled out, or the missing attachment 0 leaves the
// framebuffer incomplete.
void set_draw_buffers(uint32_t fbo, const std::vector<GLenum> &draw_buffers) {
    if (draw_buffers.empty()) {
        glNamedFramebufferDrawBuffers(fbo, 0, nullptr);
        glNamedFramebufferReadBuffer(fbo, GL_NONE);
        return;
    }

    glNamedFramebufferDrawBuffers(
        fbo, static_cast<GLsizei>(draw_buffers.size()), draw_buffers.data());
}
} // namespace

OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification &spec)
    : spec_(spec) {
    invalidate();
}

OpenGLFrameBuffer::~OpenGLFrameBuffer() {
    destroy();
}

void OpenGLFrameBuffer::invalidate() {
    destroy();

    const auto attachment_count = spec_.color_formats.size();
    color_attachments_.resize(attachment_count);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        color_attachments_[i].format = spec_.color_formats[i];
    }
    depth_.format = spec_.depth_format;

    if (spec_.num_samples > 1) {
        invalidate_multisampled();
        return;
    }

    create_color_targets();

    glCreateFramebuffers(1, &fbo_);
    std::vector<GLenum> draw_buffers;
    draw_buffers.reserve(attachment_count);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        const GLenum point = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        glNamedFramebufferTexture(
            fbo_, point, color_attachments_[i].texture, 0);
        draw_buffers.push_back(point);
    }
    set_draw_buffers(fbo_, draw_buffers);

    create_depth_target(1);
}

void OpenGLFrameBuffer::create_color_targets() {
    for (ColorAttachment &attachment : color_attachments_) {
        attachment.texture =
            create_color_texture(attachment.format, spec_.width, spec_.height);
    }
}

void OpenGLFrameBuffer::create_depth_target(int samples) {
    if (depth_.format == FrameBufferDepthFormat::None) {
        return;
    }

    if (depth_.format == FrameBufferDepthFormat::Depth32F) {
        IVX_CORE_ASSERT(samples == 1,
                        "A sampleable depth attachment cannot be multisampled");

        const bool cube = spec_.depth_texture_type == DepthTextureType::CubeMap;
        IVX_CORE_ASSERT(!cube || spec_.width == spec_.height,
                        "Depth cubemap must be square");
        glCreateTextures(
            cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, 1, &depth_.texture);
        glTextureStorage2D(depth_.texture,
                           1,
                           GL_DEPTH_COMPONENT32F,
                           spec_.width,
                           spec_.height);

        const GLint filter = spec_.depth_compare ? GL_LINEAR : GL_NEAREST;
        glTextureParameteri(depth_.texture, GL_TEXTURE_MIN_FILTER, filter);
        glTextureParameteri(depth_.texture, GL_TEXTURE_MAG_FILTER, filter);
        if (spec_.depth_compare) {
            glTextureParameteri(depth_.texture,
                                GL_TEXTURE_COMPARE_MODE,
                                GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(
                depth_.texture, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
        }
        if (cube) {
            for (GLenum axis :
                 {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R})
                glTextureParameteri(depth_.texture, axis, GL_CLAMP_TO_EDGE);
            set_depth_face(0);
            return;
        }
        glTextureParameteri(
            depth_.texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(
            depth_.texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // Reverse-Z: zero is the far/empty depth, just like the clear value.
        // PCF taps outside the map must not invent a near-plane occluder.
        const GLfloat border[4]{0.f, 0.f, 0.f, 0.f};
        glTextureParameterfv(depth_.texture, GL_TEXTURE_BORDER_COLOR, border);

        glNamedFramebufferTexture(fbo_, GL_DEPTH_ATTACHMENT, depth_.texture, 0);
        return;
    }

    // Depth24Stencil8: private to the framebuffer, never sampled.
    glCreateRenderbuffers(1, &depth_.renderbuffer);
    if (samples > 1) {
        glNamedRenderbufferStorageMultisample(depth_.renderbuffer,
                                              static_cast<GLsizei>(samples),
                                              GL_DEPTH24_STENCIL8,
                                              spec_.width,
                                              spec_.height);
    } else {
        glNamedRenderbufferStorage(depth_.renderbuffer,
                                   GL_DEPTH24_STENCIL8,
                                   spec_.width,
                                   spec_.height);
    }
    glNamedFramebufferRenderbuffer(fbo_,
                                   GL_DEPTH_STENCIL_ATTACHMENT,
                                   GL_RENDERBUFFER,
                                   depth_.renderbuffer);
}

void OpenGLFrameBuffer::invalidate_multisampled() {
    const auto attachment_count = color_attachments_.size();
    const GLsizei samples = static_cast<GLsizei>(spec_.num_samples);

    // The single-sampled copies every reader goes through; they are never drawn
    // into, only blitted to.
    create_color_targets();

    // The draw target: multisampled colour renderbuffers plus multisampled
    // depth. A renderbuffer is enough for the multisampled copies - they are
    // only ever a source for the resolve blit, never sampled by a shader.
    glCreateFramebuffers(1, &fbo_);

    std::vector<GLenum> draw_buffers;
    draw_buffers.reserve(attachment_count);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        ColorAttachment &attachment = color_attachments_[i];
        glCreateRenderbuffers(1, &attachment.renderbuffer);
        glNamedRenderbufferStorageMultisample(
            attachment.renderbuffer,
            samples,
            color_format_of(attachment.format).internal_format,
            spec_.width,
            spec_.height);

        const GLenum point = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        glNamedFramebufferRenderbuffer(
            fbo_, point, GL_RENDERBUFFER, attachment.renderbuffer);
        draw_buffers.push_back(point);
    }
    set_draw_buffers(fbo_, draw_buffers);

    create_depth_target(static_cast<int>(samples));

    // The resolve destination, attached to the single-sampled textures.
    glCreateFramebuffers(1, &resolve_fbo_);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        glNamedFramebufferTexture(resolve_fbo_,
                                  GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                  color_attachments_[i].texture,
                                  0);
    }
    set_draw_buffers(resolve_fbo_, draw_buffers);

    IVX_CORE_ASSERT(
        glCheckNamedFramebufferStatus(resolve_fbo_, GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE,
        "Resolve framebuffer is incomplete!");

    // The driver may reduce the sample count (or clamp it to GL_MAX_SAMPLES),
    // so verify what was actually allocated instead of trusting the request: a
    // silent fall back to 1 sample would look like "MSAA does nothing" rather
    // than an error.
    if (depth_.renderbuffer != 0) {
        GLint allocated_samples = 0;
        glGetNamedRenderbufferParameteriv(
            depth_.renderbuffer, GL_RENDERBUFFER_SAMPLES, &allocated_samples);
        IVX_CORE_ASSERT(allocated_samples == samples,
                        "Multisampled framebuffer got {} samples instead of "
                        "the requested {}",
                        allocated_samples,
                        static_cast<int>(samples));
    }
}

void OpenGLFrameBuffer::resolve() const {
    if (resolve_fbo_ == 0) {
        return;
    }

    // One blit per attachment: glBlitFramebuffer resolves the read
    // framebuffer's *read buffer*, so both sides have to be pointed at
    // attachment i each time.
    for (std::size_t i = 0; i < color_attachments_.size(); ++i) {
        const GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        glNamedFramebufferReadBuffer(fbo_, attachment);
        glNamedFramebufferDrawBuffer(resolve_fbo_, attachment);
        // Integer targets need GL_NEAREST (picking one sample); colour formats
        // can average.
        const GLenum filter =
            color_format_of(color_attachments_[i].format).integer ? GL_NEAREST
                                                                  : GL_LINEAR;
        glBlitNamedFramebuffer(fbo_,
                               resolve_fbo_,
                               0,
                               0,
                               static_cast<GLint>(spec_.width),
                               static_cast<GLint>(spec_.height),
                               0,
                               0,
                               static_cast<GLint>(spec_.width),
                               static_cast<GLint>(spec_.height),
                               GL_COLOR_BUFFER_BIT,
                               filter);
    }

    // Leave the multisampled framebuffer ready for the next frame's draws.
    if (!color_attachments_.empty()) {
        glNamedFramebufferReadBuffer(fbo_, GL_COLOR_ATTACHMENT0);
    }
}

void OpenGLFrameBuffer::destroy() {
    if (resolve_fbo_ != 0) {
        glDeleteFramebuffers(1, &resolve_fbo_);
        resolve_fbo_ = 0;
    }
    if (fbo_ != 0) {
        glDeleteFramebuffers(1, &fbo_);
        fbo_ = 0;
    }

    for (ColorAttachment &attachment : color_attachments_) {
        if (attachment.texture != 0) {
            glDeleteTextures(1, &attachment.texture);
        }
        if (attachment.renderbuffer != 0) {
            glDeleteRenderbuffers(1, &attachment.renderbuffer);
        }
    }
    color_attachments_.clear();

    if (depth_.texture != 0) {
        glDeleteTextures(1, &depth_.texture);
    }
    if (depth_.renderbuffer != 0) {
        glDeleteRenderbuffers(1, &depth_.renderbuffer);
    }
    depth_ = DepthAttachment{};
}

uint32_t OpenGLFrameBuffer::get_color_attachment(uint32_t index) const {
    if (index >= color_attachments_.size()) {
        return 0;
    }
    return color_attachments_[index].texture;
}

void OpenGLFrameBuffer::bind_color_attachment(uint32_t slot,
                                              uint32_t index) const {
    glBindTextureUnit(slot, get_color_attachment(index));
}

uint32_t OpenGLFrameBuffer::get_depth_attachment() const {
    return depth_.texture;
}

void OpenGLFrameBuffer::bind_depth_attachment(uint32_t slot) const {
    glBindTextureUnit(slot, depth_.texture);
}

void OpenGLFrameBuffer::set_depth_face(uint32_t face) {
    IVX_CORE_ASSERT(spec_.depth_texture_type == DepthTextureType::CubeMap &&
                        depth_.texture != 0 && face < 6,
                    "Invalid depth cubemap face");
    if (spec_.depth_texture_type != DepthTextureType::CubeMap ||
        depth_.texture == 0 || face >= 6)
        return;
    glNamedFramebufferTextureLayer(
        fbo_, GL_DEPTH_ATTACHMENT, depth_.texture, 0, face);
    IVX_CORE_ASSERT(glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER) ==
                        GL_FRAMEBUFFER_COMPLETE,
                    "Depth cubemap framebuffer is incomplete");
}

uint32_t
OpenGLFrameBuffer::read_pixel(uint32_t index, uint32_t x, uint32_t y) const {
    if (index >= color_attachments_.size() || x >= spec_.width ||
        y >= spec_.height) {
        return 0;
    }

    const ColorAttachment &attachment = color_attachments_[index];
    // glGetTextureSubImage reads straight out of the texture: no read
    // framebuffer binding to save and restore, and it cannot disturb the draw
    // state mid-frame.
    if (color_format_of(attachment.format).integer) {
        std::uint32_t value = 0;
        glGetTextureSubImage(attachment.texture,
                             0,
                             static_cast<GLint>(x),
                             static_cast<GLint>(y),
                             0,
                             1,
                             1,
                             1,
                             GL_RED_INTEGER,
                             GL_UNSIGNED_INT,
                             sizeof(value),
                             &value);
        return value;
    }

    std::uint8_t rgba[4]{};
    glGetTextureSubImage(attachment.texture,
                         0,
                         static_cast<GLint>(x),
                         static_cast<GLint>(y),
                         0,
                         1,
                         1,
                         1,
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         sizeof(rgba),
                         rgba);
    return rgba[0];
}

void OpenGLFrameBuffer::clear(const glm::vec4 &clear_color,
                              bool clear_color_attachments,
                              bool clear_depth) {
    if (clear_color_attachments) {
        for (std::size_t i = 0; i < color_attachments_.size(); ++i) {
            if (color_format_of(color_attachments_[i].format).integer) {
                const GLuint zero[4]{0u, 0u, 0u, 0u};
                glClearNamedFramebufferuiv(
                    fbo_, GL_COLOR, static_cast<GLint>(i), zero);
            } else {
                glClearNamedFramebufferfv(fbo_,
                                          GL_COLOR,
                                          static_cast<GLint>(i),
                                          glm::value_ptr(clear_color));
            }
        }
    }

    if (!clear_depth) {
        return;
    }

    // Reverse-Z with GL_GREATER: depth 1.0 is the *near* plane and 0.0 the far
    // one, so the clear value is 0.0 - the same convention
    // OpenGLRendererAPI::clear() uses. Clearing to 1.0 would put every fragment
    // "behind" the cleared plane and the whole scene would be discarded by the
    // depth test.
    if (depth_.texture != 0) {
        // A pure depth attachment cannot take the combined DEPTH_STENCIL clear.
        const GLfloat far_depth = 0.f;
        glClearNamedFramebufferfv(fbo_, GL_DEPTH, 0, &far_depth);
    } else if (depth_.renderbuffer != 0) {
        glClearNamedFramebufferfi(fbo_, GL_DEPTH_STENCIL, 0, 0.f, 0);
    }
}

void OpenGLFrameBuffer::bind() {
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_fbo_);
    glGetIntegerv(GL_VIEWPORT, previous_viewport_);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glViewport(0,
               0,
               static_cast<GLsizei>(spec_.width),
               static_cast<GLsizei>(spec_.height));
}

void OpenGLFrameBuffer::unbind() {
    // Resolve first: the multisampled attachments cannot be sampled, so
    // everything that reads this framebuffer (ImGui's image, read_pixel) goes
    // through the single-sampled copies.
    resolve();

    glBindFramebuffer(GL_FRAMEBUFFER, previous_fbo_);

    glViewport(previous_viewport_[0],
               previous_viewport_[1],
               previous_viewport_[2],
               previous_viewport_[3]);
}

void OpenGLFrameBuffer::resize(uint32_t width, uint32_t height) {
    spec_.width = width;
    spec_.height = height;

    // invalidate() releases the previous attachments, so the new size cannot
    // leak the old textures.
    invalidate();
}
} // namespace ivx
