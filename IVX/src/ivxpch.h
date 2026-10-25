#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <ranges>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ivx/core/base.h"
#include "ivx/core/logger.h"
#include "ivx/debug/instrumentor.h"

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
