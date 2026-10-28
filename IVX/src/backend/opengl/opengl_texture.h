#pragma once

#include "ivx/render/texture.h"
#include <cstdint>
#include <glad/glad.h>
#include <string>

namespace ivx {
class OpenGLTexture2D : public Texture2D {
  public:
    OpenGLTexture2D(uint32_t width, uint32_t height);
    OpenGLTexture2D(const std::filesystem::path &filepath);
    ~OpenGLTexture2D();

    virtual uint32_t get_width() const override {
        return width_;
    }

    virtual uint32_t get_height() const override {
        return height_;
    }

    virtual uint32_t get_id() const override {
        return texture_;
    }

    virtual void set_data(const void *data, uint32_t size) override;

    virtual void bind(uint32_t slot = 0) const override;

  private:
    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t texture_{0};
    GLenum internal_format;
    GLenum data_format;
};
} // namespace ivx
