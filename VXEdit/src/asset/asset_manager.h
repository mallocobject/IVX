#pragma once

#include "ivx/core/ref.hpp"
#include "ivx/render/texture.h"

#include <filesystem>

namespace ivx {

// Path keyed cache for assets that live on disk, owned by the editor: the
// engine itself has no asset database yet, it only knows how to create a
// texture from a file.
class AssetManager {
  public:
    // Returns nullptr for a path that is not a supported image. Cached by
    // absolute path.
    static Ref<Texture2D> load_texture(const std::filesystem::path &path);

    // Drops every cached handle (scene close, shutdown).
    static void clear();
};

} // namespace ivx