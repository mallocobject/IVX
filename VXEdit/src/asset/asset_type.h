#pragma once

#include <cstdint>
#include <filesystem>

namespace ivx {

// What a browser tile can represent. Anything that is not a folder, texture,
// scene or mesh is kUnknown and shows as a plain file tile.
enum class AssetType : uint8_t {
    kUnknown = 0,
    kDirectory,
    kTexture,
    kScene,
    kMesh,
};

AssetType asset_type_from_path(const std::filesystem::path &path);

// Human readable type name, used for the fallback tile and tooltips.
const char *asset_type_name(AssetType type);

} // namespace ivx