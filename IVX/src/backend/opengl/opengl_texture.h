#pragma once

#include "ivx/render/texture.h"
#include <cstdint>
#include <string>

namespace ivx {
class OpenGLTexture2D : public Texture2D {
  public:
    OpenGLTexture2D(const std::filesystem::path &filepath);
    ~OpenGLTexture2D();

    virtual uint32_t get_width() const override {
        return width_;
    }

    virtual uint32_t get_height() const override {
        return height_;
    }

    virtual void bind(uint32_t slot = 0) const override;

  private:
    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t texture_{0};
};
} // namespace ivx