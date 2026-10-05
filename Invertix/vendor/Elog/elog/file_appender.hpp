#pragma once

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#ifdef _WIN32
#define FWRITE_UNLOCKED _fwrite_nolock
#else
#define FWRITE_UNLOCKED fwrite_unlocked
#endif

namespace elog::details {
	inline void throw_system_error(const std::string& operation) {
		throw std::system_error(errno, std::system_category(), operation);
	}

	inline void throw_runtime_error(const std::string& message) {
		throw std::runtime_error(message);
	}


	inline std::filesystem::path to_platform_path(const std::string& utf8_path) {
#ifdef _WIN32
		return std::filesystem::path(std::u8string(
			reinterpret_cast<const char8_t*>(utf8_path.data()), utf8_path.size()));
#else
		return std::filesystem::path(utf8_path);
#endif
	}


	inline FILE* open_append_binary(const std::string& utf8_path) {
#ifdef _WIN32
		return _wfopen(to_platform_path(utf8_path).c_str(), L"ab");
#else
		return std::fopen(to_platform_path(utf8_path).c_str(), "ab");
#endif
	}

	class FileAppender {
	public:
		explicit FileAppender(std::string path) : path_(std::move(path)) {
			file_ = open_append_binary(path_);
			if (!file_) {
				throw_system_error("Failed to open log file");
			}
		}

		~FileAppender() {
			if (file_) {

				fclose(file_);
				file_ = nullptr;
			}
		}

		FileAppender(const FileAppender&) = delete;

		size_t written_bytes() const noexcept {
			return written_bytes_;
		}

		void reset_written_bytes() noexcept {
			written_bytes_ = 0;
		}

		void append(std::string_view data) {
			append(data.data(), data.size());
		}

		void append(const char* data, size_t len);

		void flush();

	private:
		std::string path_;
		FILE* file_{ nullptr };
		size_t written_bytes_{ 0 };
	};

	inline void FileAppender::append(const char* data, size_t len) {
		if (!data || !file_ || len == 0) {
			return;
		}

		const size_t written = FWRITE_UNLOCKED(data, 1, len, file_);

		if (written != len) {
			if (ferror(file_)) {
				throw_system_error("Failed to write log file");
			}
			throw_runtime_error("Partial write to log file");
		}

		written_bytes_ += len;
	}

	inline void FileAppender::flush() {
		if (!file_) {
			return;
		}

		if (fflush(file_) != 0) {
			throw_system_error("Failed to flush log file");
		}
	}
} // namespace elog::details
