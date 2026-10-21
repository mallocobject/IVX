#pragma once

#include "glm/ext/vector_float2.hpp"
#include "ivx/core/ref_counted.hpp"
#include "texture.h"

namespace ivx {
class SubTexture2D : public RefCounted {
  public:
    SubTexture2D(const Ref<Texture2D> &texture,
                 const glm::vec2 &min,
                 const glm::vec2 &max);

    Ref<Texture2D> get_texture() const {
        return texture_;
    }

    const glm::vec2 *get_tex_coords() const {
        return tex_coords_;
    }

    static Ref<SubTexture2D> create(const Ref<Texture2D> &texture,
                                    const glm::vec2 &offsets,
                                    const glm::vec2 &cell_size,
                                    const glm::vec2 &sprite_offsets = {1.f,
                                                                       1.f});

  private:
    Ref<Texture2D> texture_;
    glm::vec2 tex_coords_[4];
};
} // namespace ivx