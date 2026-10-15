#include "ivxpch.h"
#include "opengl_texture.h"

#include "ivx/core/base.h"
#include <cstdint>
#include <glad/glad.h>
#include <stb_image.h>

namespace ivx {
namespace {
static GLenum channels_to_internal_format(int channels) {
    switch (channels) {
    case 1:
        return GL_R8;
    case 2:
        return GL_RG8;
    case 3:
        return GL_RGB8;
    case 4:
        return GL_RGBA8;
    default:
        IVX_CORE_ASSERT(false, "Unsupported channel count!");
        return GL_RGB8;
    }
}

static GLenum channels_to_format(int channels) {
    switch (channels) {
    case 1:
        return GL_RED;
    case 2:
        return GL_RG;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        IVX_CORE_ASSERT(false, "Unsupported channel count!");
        return GL_RGB;
    }
}
} // namespace

OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path &filepath) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc *data =
        stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
    IVX_CORE_ASSERT(data, "Failed to load image: {}", filepath.string());

    width_ = static_cast<uint32_t>(width);
    height_ = static_cast<uint32_t>(height);

    const GLenum internal_format = channels_to_internal_format(channels);
    const GLenum format = channels_to_format(channels);

    glCreateTextures(GL_TEXTURE_2D, 1, &texture_);
    glTextureStorage2D(texture_, 1, internal_format, width, height);

    glTextureParameteri(texture_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(
        texture_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureSubImage2D(
        texture_, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);

    // generate mipmap(DSA)
    glGenerateTextureMipmap(texture_);

    stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    glDeleteTextures(1, &texture_);
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    glBindTextureUnit(slot, texture_);
}
} // namespace ivx
