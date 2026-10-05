#pragma once

#include "invertix/core.h"
#include <cstddef>      // size_t
#include <cstdint>      // std::uint8_t
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <format>       // std::format / std::format_string
#include <chrono>       // std::chrono::seconds
#include <utility>      // std::forward

namespace invertix {
#define FOREACH_LOG_LEVEL(f)                                              \
    f(TRACE) f(DEBUG) f(INFO) f(WARN) f(ERROR) f(FATAL)

	enum class LogLevel : std::uint8_t {
#define _FUNCTION(name) name,
		FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
	};


	IVX_API void log_message(LogLevel lv, std::string_view who, std::string_view msg);
	IVX_API void set_log_threshold(LogLevel lv);
	IVX_API void set_log_path(const std::string& dir,
		const std::string& prefix,
		size_t roll_size = 100 * 1024 * 1024,
		std::chrono::seconds flush_interval = std::chrono::seconds(3),
		size_t check_per_count = 1024);

	template <typename... Args>
	void log_fmt(LogLevel lv, std::string_view who,
		std::format_string<Args...> fmt, Args&&... args) {
		log_message(lv, who, std::format(fmt, std::forward<Args>(args)...));
	}
}


// client log macros
#define IVX_TRACE(...) ::invertix::log_fmt(::invertix::LogLevel::TRACE, "APP", __VA_ARGS__)
#define IVX_DEBUG(...) ::invertix::log_fmt(::invertix::LogLevel::DEBUG, "APP", __VA_ARGS__)
#define IVX_INFO(...) ::invertix::log_fmt(::invertix::LogLevel::INFO, "APP", __VA_ARGS__)
#define IVX_WARN(...) ::invertix::log_fmt(::invertix::LogLevel::WARN, "APP", __VA_ARGS__)
#define IVX_ERROR(...) ::invertix::log_fmt(::invertix::LogLevel::ERROR, "APP", __VA_ARGS__)
#define IVX_FATAL(...) ::invertix::log_fmt(::invertix::LogLevel::FATAL, "APP", __VA_ARGS__)

// core log macros
#define IVX_CORE_TRACE(...) ::invertix::log_fmt(::invertix::LogLevel::TRACE, "IVX", __VA_ARGS__)
#define IVX_CORE_DEBUG(...) ::invertix::log_fmt(::invertix::LogLevel::DEBUG, "IVX", __VA_ARGS__)
#define IVX_CORE_INFO(...) ::invertix::log_fmt(::invertix::LogLevel::INFO, "IVX", __VA_ARGS__)
#define IVX_CORE_WARN(...) ::invertix::log_fmt(::invertix::LogLevel::WARN, "IVX", __VA_ARGS__)
#define IVX_CORE_ERROR(...) ::invertix::log_fmt(::invertix::LogLevel::ERROR, "IVX", __VA_ARGS__)
#define IVX_CORE_FATAL(...) ::invertix::log_fmt(::invertix::LogLevel::FATAL, "IVX", __VA_ARGS__)


