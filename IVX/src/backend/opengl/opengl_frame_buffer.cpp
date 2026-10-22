#include "ivxpch.h"
#include "opengl_frame_buffer.h"

#include <cstdint>
#include <glad/glad.h>

namespace ivx {
OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification &spec)
    : spec_(spec) {
    invalidate();
}

OpenGLFrameBuffer::~OpenGLFrameBuffer() {
    clear();
}

void OpenGLFrameBuffer::invalidate() {
    if (spec_.width == 0 || spec_.height == 0) {
        return;
    }

    if (fbo_ != 0) {
        clear();
    }

    glCreateFramebuffers(1, &fbo_);

    // color attachment
    glCreateTextures(GL_TEXTURE_2D, 1, &color_tex_);
    glTextureStorage2D(color_tex_, 1, GL_RGBA8, spec_.width, spec_.height);
    glTextureParameteri(color_tex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(color_tex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(fbo_, GL_COLOR_ATTACHMENT0, color_tex_, 0);

    // depth attachment
    glCreateTextures(GL_TEXTURE_2D, 1, &depth_tex_);
    glTextureStorage2D(
        depth_tex_, 1, GL_DEPTH24_STENCIL8, spec_.width, spec_.height);
    glTextureParameteri(depth_tex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(depth_tex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(fbo_, GL_DEPTH_STENCIL_ATTACHMENT, depth_tex_, 0);

    IVX_CORE_ASSERT(glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER) ==
                        GL_FRAMEBUFFER_COMPLETE,
                    "Framebuffer is incomplete!");
}

void OpenGLFrameBuffer::clear() {
    glDeleteFramebuffers(1, &fbo_);
    glDeleteTextures(1, &color_tex_);
    glDeleteTextures(1, &depth_tex_);
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
    glBindFramebuffer(GL_FRAMEBUFFER, previous_fbo_);

    glViewport(previous_viewport_[0],
               previous_viewport_[1],
               previous_viewport_[2],
               previous_viewport_[3]);
}

void OpenGLFrameBuffer::resize(uint32_t width, uint32_t height) {
    spec_.width = width;
    spec_.height = height;
    invalidate();
}
} // namespace ivx