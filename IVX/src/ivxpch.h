#pragma once

#include <cstdint>

#include <chrono>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include <algorithm>
#include <ranges>

#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ivx/logger.h"

#ifdef IVX_PLATFORM_WINDOWS
#include <Windows.h>
#undef ERROR
#undef min
#undef max
#endif

namespace ivx {
namespace rg = std::ranges;
namespace vws = std::views;
} // namespace ivx
