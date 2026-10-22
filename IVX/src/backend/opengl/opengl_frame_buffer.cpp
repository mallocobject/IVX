#include "ivxpch.h"
#include "opengl_frame_buffer.h"

#include <glad/glad.h>

namespace ivx {
OpenGLFrameBuffer::OpenGLFrameBuffer(const FrameBufferSpecification &spec)
    : spec_(spec) {
    invalidate();
}

OpenGLFrameBuffer::~OpenGLFrameBuffer() {
    glDeleteFramebuffers(1, &fbo_);
}

void OpenGLFrameBuffer::invalidate() {
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

void OpenGLFrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    ;
}

void OpenGLFrameBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
} // namespace ivx