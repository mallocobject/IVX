#include "asset_type.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace ivx {
namespace {
std::string lowercase_extension(const std::filesystem::path &path) {
    std::string extension = path.extension().string();
    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension;
}
} // namespace

AssetType asset_type_from_path(const std::filesystem::path &path) {
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
        return AssetType::kDirectory;
    }

    const std::string extension = lowercase_extension(path);
    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
        extension == ".tga" || extension == ".bmp") {
        return AssetType::kTexture;
    }
    if (extension == ".ivx") {
        return AssetType::kScene;
    }
    if (extension == ".gltf" || extension == ".glb" || extension == ".obj" ||
        extension == ".fbx") {
        return AssetType::kMesh;
    }
    return AssetType::kUnknown;
}

const char *asset_type_name(AssetType type) {
    switch (type) {
    case AssetType::kDirectory:
        return "Folder";
    case AssetType::kTexture:
        return "Texture";
    case AssetType::kScene:
        return "Scene";
    case AssetType::kMesh:
        return "Mesh";
    case AssetType::kUnknown:
    default:
        return "File";
    }
}

} // namespace ivx