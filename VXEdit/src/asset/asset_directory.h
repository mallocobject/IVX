#pragma once

#include "asset_type.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace ivx {

struct AssetEntry {
    std::filesystem::path path;
    std::string display_name;
    AssetType type{AssetType::kUnknown};
    bool is_directory{false};
    std::uintmax_t size_bytes{0};
};

// One listing. Nothing is cached: the browser rescans every frame, which keeps
// it in sync with the disk without a watcher or timestamps.
struct AssetDirectory {
    std::filesystem::path path;
    std::vector<AssetEntry> entries;
    // Non-empty when the directory could not be read (missing, permissions).
    std::string error;
    bool truncated{false};

    static constexpr std::size_t k_max_entries = 1000;

    static AssetDirectory scan(const std::filesystem::path &directory);
};

// True when `path` is `root` itself or sits below it. Components are compared
// one by one, so "assets2" is not treated as being inside "assets".
bool is_inside(const std::filesystem::path &root,
               const std::filesystem::path &path);

} // namespace ivx