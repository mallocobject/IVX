#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/core/ref_counted.hpp"
#include <cstdint>
#include <filesystem>
#include <string>

namespace ivx {
class Texture : public RefCounted {
  public:
    virtual ~Texture() = default;

    virtual uint32_t get_width() const = 0;
    virtual uint32_t get_height() const = 0;

    virtual void set_data(const void *data, uint32_t size) = 0;

    virtual void bind(uint32_t slot = 0) const = 0;
};

class Texture2D : public Texture {
  public:
    static Ref<Texture2D> create(uint32_t width, uint32_t height);
    static Ref<Texture2D> create(const std::filesystem::path &filepath);
};
} // namespace ivx