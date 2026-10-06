#pragma once

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "elog/async_logger.hpp"
#include <atomic>
#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <source_location>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <syncstream>


#ifdef _WIN32
extern "C" {
	__declspec(dllimport) void* __stdcall GetStdHandle(unsigned long nStdHandle);
	__declspec(dllimport) int __stdcall GetConsoleMode(void* hConsoleHandle,
		unsigned long* lpMode);
	__declspec(dllimport) int __stdcall SetConsoleMode(void* hConsoleHandle,
		unsigned long dwMode);
}
#endif

namespace elog {
#define ELOG_FOREACH_LOG_LEVEL(f)                                              \
    f(TRACE) f(DEBUG) f(INFO) f(WARN) f(ERROR) f(FATAL)

	enum class LogLevel : std::uint8_t {
#define _FUNCTION(name) name,
		ELOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
	};

	namespace details {
		inline constexpr const char
			level_ansi_colors[static_cast<std::uint8_t>(LogLevel::FATAL) + 1][6] = {
				"\033[90m", "\033[36m", "\033[32m", "\033[33m", "\033[31m", "\033[35m" };

		inline constexpr std::string_view log_level_to_string(LogLevel lv) {
			switch (lv) {
#define _FUNCTION(name)                                                        \
    case LogLevel::name:                                                       \
        return #name;
				ELOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
			}
			return "UNKNOWN";
		}

		inline constexpr LogLevel log_level_from_string(std::string_view lv) {
#define _FUNCTION(name)                                                        \
    if (lv == #name)                                                           \
        return LogLevel::name;
			ELOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

				return LogLevel::INFO;
		}

		inline std::string env_utf8(const char* name) {
#ifdef _WIN32
			std::wstring wname;
			for (const char* p = name; *p != '\0'; ++p) {
				wname.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*p)));
			}
			const wchar_t* const value = ::_wgetenv(wname.c_str());
			if (value == nullptr) {
				return {};
			}
			const std::u8string utf8 = std::filesystem::path(value).u8string();
			return std::string(reinterpret_cast<const char*>(utf8.data()),
				utf8.size());
#else
			const char* const value = std::getenv(name);
			return value != nullptr ? std::string(value) : std::string{};
#endif
		}


		inline bool detect_ansi_support() noexcept {
#ifdef _WIN32
			constexpr unsigned long kStdOutputHandle =
				static_cast<unsigned long>(-11); // STD_OUTPUT_HANDLE
			constexpr unsigned long kEnableVirtualTerminalProcessing = 0x0004;

			const void* const invalid_handle =
				reinterpret_cast<const void*>(static_cast<std::intptr_t>(-1));

			void* const handle = ::GetStdHandle(kStdOutputHandle);
			if (handle == nullptr || handle == invalid_handle) {
				return false;
			}

			unsigned long mode = 0;
			if (::GetConsoleMode(handle, &mode) == 0) {
				return false;
			}
			if ((mode & kEnableVirtualTerminalProcessing) != 0) {
				return true;
			}
			return ::SetConsoleMode(handle, mode | kEnableVirtualTerminalProcessing) !=
				0;
#else
			return true;
#endif
		}


		inline bool ansi_colors_enabled() noexcept {
			static const bool enabled = detect_ansi_support();
			return enabled;
		}


		inline const char* level_color(LogLevel lv) noexcept {
			return ansi_colors_enabled()
				? level_ansi_colors[static_cast<std::uint8_t>(lv)]
				: "";
		}

		template <typename T>
		class WithSourceLocation {
		public:
			template <typename U>
				requires std::constructible_from<T, U>
			consteval WithSourceLocation(
				U&& inner, std::source_location loc = std::source_location::current())
				: inner_(std::forward<U>(inner)), loc_(std::move(loc)) {
			}

			constexpr const T& format() const noexcept {
				return inner_;
			}

			constexpr const std::source_location& location() const noexcept {
				return loc_;
			}

		private:
			T inner_;
			std::source_location loc_;
		};

		inline auto g_log_file = []() -> std::unique_ptr<AsyncLogger> {
			const std::string path = env_utf8("ELOG_PATH");
			if (!path.empty()) {
				return std::make_unique<AsyncLogger>(path, "");
			}
			return nullptr;
			}();

		inline std::atomic<LogLevel> g_log_threshold{ []() -> LogLevel {
			const std::string lv = env_utf8("ELOG_LEVEL");
			if (!lv.empty()) {
				return log_level_from_string(lv);
			}
			return LogLevel::INFO;
		}() };

		// hook function, empty implement in default case
		inline auto g_log_callback = std::function<void(LogLevel, std::string_view)>{};

		// query zone only once
		inline const std::chrono::time_zone* cached_zone() {
			static const std::chrono::time_zone* zone = std::chrono::current_zone();
			return zone;
		}

		inline void
			output_log(LogLevel lv, std::string_view who, std::string_view msg, const std::source_location& loc) {
			if (lv < g_log_threshold.load(std::memory_order_relaxed)) {
				return;
			}

			if (g_log_file == nullptr && g_log_callback) {
				g_log_callback(lv, msg);
				return;
			}

			//thread_local std::uint64_t tid =
			//	std::hash<std::thread::id>{}(std::this_thread::get_id());

			std::chrono::zoned_time now{ cached_zone(),
			std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()) };
			//std::string fmsg = std::format("{}[{}]<{}> {}:{} {}()-> {}",
			//	now,
			//	tid,
			//	log_level_to_string(lv),
			//	loc.file_name(),
			//	loc.line(),
			//	loc.function_name(),
			//	msg);

			std::string fmsg = std::format("[{:%H:%M:%S}] [{}] {}",
				now,
				who,
				msg);

			if (g_log_file) {
				fmsg += '\n';
				g_log_file->append_message(fmsg);
			}

			if (g_log_callback) {
				g_log_callback(lv, msg);
			}
			else {
				const char* color = level_color(lv);
				std::osyncstream sync_out(std::cout);
				if (color[0] != '\0') {
					sync_out << color << fmsg << "\033[0m" << std::endl;
				}
				else {
					sync_out << fmsg << std::endl;
				}
			}
		}
	} // namespace details

	inline void set_log_path(const std::string& dir,
		const std::string& prefix,
		size_t roll_size,
		std::chrono::seconds flush_interval,
		size_t check_per_count) {
		details::g_log_file = std::make_unique<details::AsyncLogger>(
			dir, prefix, roll_size, flush_interval, check_per_count);
	}

	inline void set_log_threshold(LogLevel lv) {
		details::g_log_threshold.store(lv, std::memory_order_relaxed);
	}


	template <typename... Args>
	constexpr void log_fmt(LogLevel lv, std::string_view who,
		details::WithSourceLocation<std::format_string<Args...>> fmt,
		Args &&...args) {
		if (lv < details::g_log_threshold) {
			return;
		}

		auto msg = std::vformat(fmt.format().get(), std::make_format_args(args...));

		details::output_log(lv, who, msg, fmt.location());
	}

#define _FUNCTION(name)                                                        \
    template <typename... Args>                                                \
    constexpr void LOG_##name(												   \
		std::string_view who,												   \
        details::WithSourceLocation<std::format_string<Args...>> fmt,          \
        Args &&...args) {                                                      \
        return log_fmt(                                                            \
            LogLevel::name, who, std::move(fmt), std::forward<Args>(args)...); \
    }
	ELOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
} // namespace elog
