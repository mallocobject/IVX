#include "asset_manager.h"

#include "ivx/core/logger.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>

namespace ivx {
namespace {
std::unordered_map<std::string, Ref<Texture2D>> texture_cache;

bool is_image(const std::filesystem::path &path) {
    std::string extension = path.extension().string();
    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
           extension == ".tga" || extension == ".bmp";
}
} // namespace

Ref<Texture2D> AssetManager::load_texture(const std::filesystem::path &path) {
    if (!is_image(path)) {
        IVX_CORE_WARN("AssetManager: '{}' is not a supported image",
                      path.string());
        return nullptr;
    }

    std::error_code ec;
    const std::filesystem::path canonical =
        std::filesystem::weakly_canonical(path, ec);
    const std::string key = (ec ? path : canonical).string();

    if (const auto it = texture_cache.find(key); it != texture_cache.end()) {
        return it->second;
    }

    Ref<Texture2D> texture = Texture2D::create(path);
    texture_cache.emplace(key, texture);
    return texture;
}

void AssetManager::clear() {
    texture_cache.clear();
}

} // namespace ivx