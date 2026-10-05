#pragma once

#include <cstdint>

#include <iostream>
#include <format>
#include <string>
#include <functional>
#include <string_view>
#include <chrono>
#include <memory>
#include <type_traits>

#include <algorithm>
#include <ranges>

#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <queue>

#include "invertix/logger.h"

#ifdef IVX_PLATFORM_WINDOWS
#include <Windows.h>
#undef ERROR
#undef min
#undef max
#endif

namespace invertix {
	namespace rg = std::ranges;
	namespace vws = std::views;
}

