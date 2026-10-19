#include "ivxpch.h"
#include "opengl_texture.h"

#include "ivx/core/base.h"
#include "ivx/debug/instrumentor.h"
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

OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
    : width_(width), height_(height), internal_format(GL_RGBA8),
      data_format(GL_RGBA) {
    IVX_PROFILE_FUNCTION();

    glCreateTextures(GL_TEXTURE_2D, 1, &texture_);
    glTextureStorage2D(texture_, 1, internal_format, width, height);

    glTextureParameteri(texture_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(
        texture_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path &filepath) {
    IVX_PROFILE_FUNCTION();

    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc *data = nullptr;
    {
        IVX_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const "
                          "std::filesystem::path &)");
        data =
            stbi_load(filepath.string().c_str(), &width, &height, &channels, 0);
    }
    IVX_CORE_ASSERT(data, "Failed to load image: {}", filepath.string());

    width_ = static_cast<uint32_t>(width);
    height_ = static_cast<uint32_t>(height);

    internal_format = channels_to_internal_format(channels);
    data_format = channels_to_format(channels);

    glCreateTextures(GL_TEXTURE_2D, 1, &texture_);
    glTextureStorage2D(texture_, 1, internal_format, width, height);

    glTextureParameteri(texture_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture_, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(
        texture_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureSubImage2D(
        texture_, 0, 0, 0, width, height, data_format, GL_UNSIGNED_BYTE, data);

    // generate mipmap(DSA)
    glGenerateTextureMipmap(texture_);

    stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    IVX_PROFILE_FUNCTION();

    glDeleteTextures(1, &texture_);
}

void OpenGLTexture2D::set_data(void *data, uint32_t size) {
    IVX_PROFILE_FUNCTION();

    uint32_t bpp = data_format == GL_RGBA ? 4 : 3;
    IVX_CORE_ASSERT(size == width_ * height_ * bpp,
                    "Data must be entire texture!");
    glTextureSubImage2D(texture_,
                        0,
                        0,
                        0,
                        width_,
                        height_,
                        data_format,
                        GL_UNSIGNED_BYTE,
                        data);
}

void OpenGLTexture2D::bind(uint32_t slot) const {
    IVX_PROFILE_FUNCTION();

    glBindTextureUnit(slot, texture_);
}
} // namespace ivx
