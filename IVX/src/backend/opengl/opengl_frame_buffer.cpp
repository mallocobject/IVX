#include "ivxpch.h"
#include "opengl_frame_buffer.h"

#include "ivx/core/logger.h"
#include <cstdint>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace ivx {
namespace {
// Internal texture format for a colour attachment. The integer one has to stay
// integer: blending and filtering are meaningless for ids, and GL would
// silently produce garbage.
GLenum internal_format_of(FrameBufferTextureFormat format) {
    switch (format) {
    case FrameBufferTextureFormat::RGBA16F:
        return GL_RGBA16F;
    case FrameBufferTextureFormat::RED_INTEGER:
        return GL_R32UI;
    case FrameBufferTextureFormat::None:
        // A "None" entry is treated as ordinary colour rather than failing the
        // whole framebuffer: it is what a default-constructed spec asks for.
    case FrameBufferTextureFormat::RGBA8:
    default:
        return GL_RGBA8;
    }
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

    const auto attachment_count =
        static_cast<std::size_t>(spec_.attachments.size());
    color_formats_.resize(attachment_count);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        color_formats_[i] = spec_.attachments[i].format;
    }

    color_attachments_.assign(attachment_count, 0);
    glCreateTextures(GL_TEXTURE_2D,
                     static_cast<GLsizei>(attachment_count),
                     color_attachments_.data());

    for (std::size_t i = 0; i < attachment_count; ++i) {
        const bool integer =
            color_formats_[i] == FrameBufferTextureFormat::RED_INTEGER;
        glTextureStorage2D(color_attachments_[i],
                           1,
                           internal_format_of(color_formats_[i]),
                           spec_.width,
                           spec_.height);
        // Integer targets must not be filtered, or read_pixel() gets
        // interpolated ids.
        const GLint filter = integer ? GL_NEAREST : GL_LINEAR;
        glTextureParameteri(
            color_attachments_[i], GL_TEXTURE_MIN_FILTER, filter);
        glTextureParameteri(
            color_attachments_[i], GL_TEXTURE_MAG_FILTER, filter);
    }

    if (spec_.num_samples > 1) {
        invalidate_multisampled();
        return;
    }

    glCreateFramebuffers(1, &fbo_);

    std::vector<GLenum> draw_buffers(attachment_count, GL_NONE);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        glNamedFramebufferTexture(fbo_,
                                  GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                  color_attachments_[i],
                                  0);
        draw_buffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
    }

    // Without this every attachment past the first is silently never written:
    // GL only enables COLOR_ATTACHMENT0 on a freshly created framebuffer.
    glNamedFramebufferDrawBuffers(
        fbo_, static_cast<GLsizei>(attachment_count), draw_buffers.data());

    glCreateRenderbuffers(1, &depth_buffer_);
    glNamedRenderbufferStorage(
        depth_buffer_, GL_DEPTH24_STENCIL8, spec_.width, spec_.height);
    glNamedFramebufferRenderbuffer(
        fbo_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);

    IVX_CORE_ASSERT(glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER) ==
                        GL_FRAMEBUFFER_COMPLETE,
                    "Framebuffer is incomplete!");
}

void OpenGLFrameBuffer::invalidate_multisampled() {
    const auto attachment_count =
        static_cast<std::size_t>(spec_.attachments.size());
    const GLsizei samples = static_cast<GLsizei>(spec_.num_samples);

    // The draw target: multisampled colour renderbuffers plus multisampled
    // depth/stencil. A renderbuffer is enough for the multisampled copies -
    // they are only ever a source for the resolve blit, never sampled by a
    // shader.
    glCreateFramebuffers(1, &fbo_);

    multisampled_attachments_.assign(attachment_count, 0);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        glCreateRenderbuffers(1, &multisampled_attachments_[i]);
        glNamedRenderbufferStorageMultisample(
            multisampled_attachments_[i],
            samples,
            internal_format_of(color_formats_[i]),
            spec_.width,
            spec_.height);
        glNamedFramebufferRenderbuffer(fbo_,
                                       GL_COLOR_ATTACHMENT0 +
                                           static_cast<GLenum>(i),
                                       GL_RENDERBUFFER,
                                       multisampled_attachments_[i]);
    }

    std::vector<GLenum> draw_buffers(attachment_count, GL_NONE);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        draw_buffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
    }
    glNamedFramebufferDrawBuffers(
        fbo_, static_cast<GLsizei>(attachment_count), draw_buffers.data());

    glCreateRenderbuffers(1, &depth_buffer_);
    glNamedRenderbufferStorageMultisample(
        depth_buffer_, samples, GL_DEPTH24_STENCIL8, spec_.width, spec_.height);
    glNamedFramebufferRenderbuffer(
        fbo_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_);

    IVX_CORE_ASSERT(glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER) ==
                        GL_FRAMEBUFFER_COMPLETE,
                    "Multisampled framebuffer is incomplete!");

    // The resolve target holds the single-sampled textures handed out by
    // get_color_attachment(); it is never drawn into, only blitted to.
    glCreateFramebuffers(1, &resolve_fbo_);
    for (std::size_t i = 0; i < attachment_count; ++i) {
        glNamedFramebufferTexture(resolve_fbo_,
                                  GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i),
                                  color_attachments_[i],
                                  0);
    }
    glNamedFramebufferDrawBuffers(resolve_fbo_,
                                  static_cast<GLsizei>(attachment_count),
                                  draw_buffers.data());

    IVX_CORE_ASSERT(
        glCheckNamedFramebufferStatus(resolve_fbo_, GL_FRAMEBUFFER) ==
            GL_FRAMEBUFFER_COMPLETE,
        "Resolve framebuffer is incomplete!");

    // The driver may reduce the sample count (or clamp it to GL_MAX_SAMPLES),
    // so verify what was actually allocated instead of trusting the request: a
    // silent fall back to 1 sample would look like "MSAA does nothing" rather
    // than an error.
    GLint allocated_samples = 0;
    glGetNamedRenderbufferParameteriv(
        depth_buffer_, GL_RENDERBUFFER_SAMPLES, &allocated_samples);
    IVX_CORE_ASSERT(
        allocated_samples == samples,
        "Multisampled framebuffer got {} samples instead of the requested {}",
        allocated_samples,
        static_cast<int>(samples));
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
            color_formats_[i] == FrameBufferTextureFormat::RED_INTEGER
                ? GL_NEAREST
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
    glNamedFramebufferReadBuffer(fbo_, GL_COLOR_ATTACHMENT0);
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
    if (!multisampled_attachments_.empty()) {
        glDeleteRenderbuffers(
            static_cast<GLsizei>(multisampled_attachments_.size()),
            multisampled_attachments_.data());
        multisampled_attachments_.clear();
    }
    if (!color_attachments_.empty()) {
        glDeleteTextures(static_cast<GLsizei>(color_attachments_.size()),
                         color_attachments_.data());
        color_attachments_.clear();
    }
    color_formats_.clear();
    if (depth_buffer_ != 0) {
        glDeleteRenderbuffers(1, &depth_buffer_);
        depth_buffer_ = 0;
    }
}

uint32_t OpenGLFrameBuffer::get_color_attachment(uint32_t index) const {
    if (index >= color_attachments_.size()) {
        return 0;
    }
    return color_attachments_[index];
}

int OpenGLFrameBuffer::read_pixel(uint32_t index,
                                  uint32_t x,
                                  uint32_t y) const {
    if (index >= color_attachments_.size() || x >= spec_.width ||
        y >= spec_.height) {
        return -1;
    }

    // glGetTextureSubImage reads straight out of the texture: no read
    // framebuffer binding to save and restore, and it cannot disturb the draw
    // state mid-frame.
    if (color_formats_[index] == FrameBufferTextureFormat::RED_INTEGER) {
        std::uint32_t value = 0;
        glGetTextureSubImage(color_attachments_[index],
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
        return static_cast<int>(value);
    }

    std::uint8_t rgba[4]{};
    glGetTextureSubImage(color_attachments_[index],
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
            if (color_formats_[i] == FrameBufferTextureFormat::RED_INTEGER) {
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

    if (clear_depth) {
        // Reverse-Z with GL_GREATER: depth 1.0 is the *near* plane and 0.0 the
        // far one, so the clear value is 0.0 - the same convention
        // OpenGLRendererAPI::clear() uses. Clearing to 1.0 would put every
        // fragment "behind" the cleared plane and the whole scene would be
        // discarded by the depth test.
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
