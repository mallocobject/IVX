#include "ivxpch.h"
#include "sub_texture2d.h"

namespace ivx {
Ref<SubTexture2D> SubTexture2D::create(const Ref<Texture2D> &texture,
                                       const glm::vec2 &offsets,
                                       const glm::vec2 &cell_size,
                                       const glm::vec2 &sprite_offsets) {
    int width = texture->get_width();
    int height = texture->get_height();

    glm::vec2 min;
    glm::vec2 max;

    min.x = offsets.x * cell_size.x / width;
    min.y = offsets.y * cell_size.y / height;

    max.x = (offsets.x + sprite_offsets.x) * cell_size.x / width;
    max.y = (offsets.y + sprite_offsets.y) * cell_size.y / height;

    return make_ref<SubTexture2D>(texture, min, max);
}

SubTexture2D::SubTexture2D(const Ref<Texture2D> &texture,
                           const glm::vec2 &min,
                           const glm::vec2 &max)
    : texture_(texture) {
    tex_coords_[0] = min;
    tex_coords_[1] = {max.x, min.y};
    tex_coords_[2] = max;
    tex_coords_[3] = {min.x, max.y};
}
} // namespace ivx