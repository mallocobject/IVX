#pragma once

#include "asset_type.h"

namespace ivx {

inline constexpr const char *k_asset_payload_type = "IVX_ASSET";

// ImGui payloads are raw memory copies, so this has to stay trivially copyable:
// no path, no string. `path` is UTF-8.
struct AssetPayload {
    AssetType type{AssetType::kUnknown};
    char path[512]{};
};

} // namespace ivx