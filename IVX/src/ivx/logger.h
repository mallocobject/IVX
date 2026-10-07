#pragma once

#include "ivx/core.h"
#include <chrono>      // std::chrono::seconds
#include <cstddef>     // size_t
#include <cstdint>     // std::uint8_t
#include <format>      // std::format / std::format_string
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward


namespace ivx {
#define FOREACH_LOG_LEVEL(f) f(TRACE) f(DEBUG) f(INFO) f(WARN) f(ERROR) f(FATAL)

enum class LogLevel : std::uint8_t {
#define _FUNCTION(name) name,
    FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

void log_message(LogLevel lv, std::string_view who, std::string_view msg);
void set_log_threshold(LogLevel lv);
void set_log_path(const std::string &dir,
                  const std::string &prefix,
                  size_t roll_size = 100 * 1024 * 1024,
                  std::chrono::seconds flush_interval = std::chrono::seconds(3),
                  size_t check_per_count = 1024);

template <typename... Args>
void log_fmt(LogLevel lv,
             std::string_view who,
             std::format_string<Args...> fmt,
             Args &&...args) {
    log_message(lv, who, std::format(fmt, std::forward<Args>(args)...));
}
} // namespace ivx

// client log macros
#define IVX_TRACE(...)                                                         \
    ::ivx::log_fmt(::ivx::LogLevel::TRACE, "APP", __VA_ARGS__)
#define IVX_DEBUG(...)                                                         \
    ::ivx::log_fmt(::ivx::LogLevel::DEBUG, "APP", __VA_ARGS__)
#define IVX_INFO(...) ::ivx::log_fmt(::ivx::LogLevel::INFO, "APP", __VA_ARGS__)
#define IVX_WARN(...) ::ivx::log_fmt(::ivx::LogLevel::WARN, "APP", __VA_ARGS__)
#define IVX_ERROR(...)                                                         \
    ::ivx::log_fmt(::ivx::LogLevel::ERROR, "APP", __VA_ARGS__)
#define IVX_FATAL(...)                                                         \
    ::ivx::log_fmt(::ivx::LogLevel::FATAL, "APP", __VA_ARGS__)

// core log macros
#define IVX_CORE_TRACE(...)                                                    \
    ::ivx::log_fmt(::ivx::LogLevel::TRACE, "IVX", __VA_ARGS__)
#define IVX_CORE_DEBUG(...)                                                    \
    ::ivx::log_fmt(::ivx::LogLevel::DEBUG, "IVX", __VA_ARGS__)
#define IVX_CORE_INFO(...)                                                     \
    ::ivx::log_fmt(::ivx::LogLevel::INFO, "IVX", __VA_ARGS__)
#define IVX_CORE_WARN(...)                                                     \
    ::ivx::log_fmt(::ivx::LogLevel::WARN, "IVX", __VA_ARGS__)
#define IVX_CORE_ERROR(...)                                                    \
    ::ivx::log_fmt(::ivx::LogLevel::ERROR, "IVX", __VA_ARGS__)
#define IVX_CORE_FATAL(...)                                                    \
    ::ivx::log_fmt(::ivx::LogLevel::FATAL, "IVX", __VA_ARGS__)
