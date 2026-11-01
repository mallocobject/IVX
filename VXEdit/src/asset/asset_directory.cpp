#include "asset_directory.h"

#include <algorithm>
#include <cctype>
#include <format>
#include <ranges>

namespace ivx {
namespace {
std::string lowercase(std::string text) {
    std::ranges::transform(text, text.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return text;
}

bool is_hidden(const std::filesystem::path &path) {
    const std::string name = path.filename().string();
    return !name.empty() && name[0] == '.';
}
} // namespace

AssetDirectory AssetDirectory::scan(const std::filesystem::path &directory) {
    AssetDirectory result;
    result.path = directory;

    std::error_code ec;
    std::filesystem::directory_iterator it{directory, ec};
    if (ec) {
        result.error = std::format(
            "Cannot read '{}': {}", directory.string(), ec.message());
        return result;
    }

    const std::filesystem::directory_iterator end;
    for (; !ec && it != end; it.increment(ec)) {
        const std::filesystem::directory_entry &entry = *it;
        if (is_hidden(entry.path())) {
            continue;
        }
        if (result.entries.size() >= k_max_entries) {
            result.truncated = true;
            break;
        }

        AssetEntry asset;
        asset.path = entry.path();
        asset.display_name = entry.path().filename().string();
        asset.type = asset_type_from_path(asset.path);
        asset.is_directory = asset.type == AssetType::kDirectory;
        if (!asset.is_directory) {
            std::error_code size_ec;
            asset.size_bytes = entry.file_size(size_ec);
        }
        result.entries.push_back(std::move(asset));
    }

    if (ec && result.error.empty()) {
        result.error = std::format(
            "Cannot read '{}': {}", directory.string(), ec.message());
    }

    // Folders first, then by lower case name so the order does not depend on
    // the locale.
    std::ranges::sort(
        result.entries, [](const AssetEntry &lhs, const AssetEntry &rhs) {
            if (lhs.is_directory != rhs.is_directory) {
                return lhs.is_directory;
            }
            return lowercase(lhs.display_name) < lowercase(rhs.display_name);
        });

    return result;
}

bool is_inside(const std::filesystem::path &root,
               const std::filesystem::path &path) {
    auto root_it = root.begin();
    for (auto path_it = path.begin(); root_it != root.end();
         ++root_it, ++path_it) {
        if (path_it == path.end() || *path_it != *root_it) {
            return false;
        }
    }
    return true;
}

} // namespace ivx
