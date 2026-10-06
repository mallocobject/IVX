#pragma once

#include "elog/file_manager.hpp"
#include "elog/log_block.hpp"
#include "elog/spsc_queue.hpp"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace elog::details {
	class AsyncLogger;

	struct ProducerCtx {
		LogBlock* take_free() {
			using Node = SpscQueue<LogBlock*>::Node;
			Node* stash = free_stash;
			if (!stash) {
				stash = free.drain();
			}
			if (!stash) {
				return nullptr;
			}
			LogBlock* blk = stash->value;
			free_stash = stash->next;
			delete stash;
			blk->clear();
			return blk;
		}

		std::uint64_t owner_id{ 0 };

		std::mutex cur_mtx;
		LogBlock* cur{ new LogBlock() };

		SpscQueue<LogBlock*> full{};
		SpscQueue<LogBlock*> free{};

		SpscQueue<LogBlock*>::Node* free_stash{ nullptr };
	};

	class AsyncLogger {
	public:
		explicit AsyncLogger(
			std::string dir,
			std::string prefix,
			size_t roll_size = 100 * 1024 * 1024,
			std::chrono::seconds flush_interval = std::chrono::seconds(3),
			size_t check_per_count = 1024) {
			thread_ = std::jthread([this,
				dir = std::move(dir),
				prefix = std::move(prefix),
				roll_size,
				flush_interval,
				check_per_count] {
					run(dir, prefix, roll_size, flush_interval, check_per_count);
				});
		}

		AsyncLogger(AsyncLogger&&) = delete;

		~AsyncLogger() {
			if (!done_.load(std::memory_order_acquire)) {
				do_done();
			}
		}

		void append_message(std::string_view msg);

		void wait_for_done() {
			if (!done_.load(std::memory_order_acquire)) {
				do_done();
			}
		}

	private:
		ProducerCtx* register_producer();

		void run(const std::string& dir,
			const std::string& prefix,
			size_t roll_size,
			std::chrono::seconds flush_interval,
			size_t check_per_count);

		void do_done() {
			{
				std::lock_guard lock(cv_mtx_);
				done_.store(true, std::memory_order_release);
			}
			cv_.notify_one();
			if (thread_.joinable()) {
				thread_.join();
			}
		}

		void stop_accepting() noexcept {
			accepting_.store(false, std::memory_order_release);
		}

		inline static std::uint64_t next_instance_id() noexcept {
			static std::atomic<std::uint64_t> seq{ 0 };
			return seq.fetch_add(1, std::memory_order_relaxed);
		}

		const std::uint64_t id_{ next_instance_id() };

		std::atomic<bool> done_{ false };
		std::atomic<bool> accepting_{ true };
		std::mutex cv_mtx_;
		std::condition_variable cv_;

		std::mutex reg_mtx_;
		std::vector<ProducerCtx*> producers_;

		std::jthread thread_;
	};

	inline ProducerCtx* AsyncLogger::register_producer() {
		auto* ctx = new ProducerCtx();
		ctx->owner_id = id_;
		{
			std::lock_guard lock(reg_mtx_);
			producers_.push_back(ctx);
		}
		return ctx;
	}

	inline void AsyncLogger::append_message(std::string_view msg) {
		if (!accepting_.load(std::memory_order_acquire)) {
			return;
		}

		thread_local ProducerCtx* ctx = nullptr;
		if (ctx == nullptr || ctx->owner_id != id_) {
			ctx = register_producer();
		}

		bool wake = false;
		{
			std::lock_guard<std::mutex> lock(ctx->cur_mtx);

			if (ctx->cur->append(msg)) {
				return;
			}


			std::string_view rest = msg;
			while (!rest.empty()) {
				const size_t space = LogBlock::kCap - ctx->cur->len;
				if (space == 0) {
					ctx->full.push(ctx->cur);
					wake = true;

					LogBlock* fresh = ctx->take_free();
					if (!fresh) {
						fresh = new LogBlock();
					}
					ctx->cur = fresh;
					continue;
				}

				const std::string_view chunk = rest.substr(0, space);
				ctx->cur->append(chunk);
				rest.remove_prefix(chunk.size());
			}
		}

		if (wake) {
			ctx->full.flush();
			cv_.notify_one();
		}
	}

	inline void AsyncLogger::run(const std::string& dir,
		const std::string& prefix,
		size_t roll_size,
		std::chrono::seconds flush_interval,
		size_t check_per_count) {
		std::unique_ptr<FileManager> out_file;
		try {
			out_file = std::make_unique<FileManager>(
				dir, prefix, roll_size, flush_interval, check_per_count);
		}
		catch (const std::exception& e) {
			// std::fprintf(stderr, "[elog] file logging disabled: %s\n", e.what());
			std::cerr << std::format("[elog] file logging disabled: {}", e.what())
				<< std::endl;
		}

		if (!out_file) {
			stop_accepting();
			return;
		}

		bool file_ok = true;

		while (true) {
			{
				std::unique_lock<std::mutex> lock(cv_mtx_);
				cv_.wait_for(lock, flush_interval, [this] {
					return done_.load(std::memory_order_acquire);
					});
			}

			if (done_.load(std::memory_order_acquire)) {
				break;
			}

			try {
				std::vector<ProducerCtx*> snapshot;
				{
					std::lock_guard lock(reg_mtx_);
					snapshot = producers_;
				}

				bool any = false;
				for (auto* ctx : snapshot) {
					for (auto* n = to_fifo<LogBlock*>(ctx->full.drain()); n;) {
						auto* node = n;
						n = n->next;
						out_file->append(node->value->data, node->value->len);
						ctx->free.push(node->value);
						delete node;
						any = true;
					}
					ctx->free.flush();

					LogBlock* taken = nullptr;
					{
						std::lock_guard<std::mutex> lock(ctx->cur_mtx);
						if (!ctx->cur->empty()) {
							taken = ctx->cur;
							ctx->cur = new LogBlock();
						}
					}
					if (taken) {
						out_file->append(taken->data, taken->len);
						ctx->free.push(taken);
						ctx->free.flush();
						any = true;
					}
				}

				if (any) {
					out_file->flush();
				}
			}
			catch (const std::exception& e) {
				std::cerr << std::format("[elog] file logging disabled: {}",
					e.what())
					<< std::endl;
				file_ok = false;
				break;
			}
		}

		stop_accepting();

		if (file_ok) {
			try {
				std::vector<ProducerCtx*> snapshot;
				{
					std::lock_guard lock(reg_mtx_);
					snapshot = producers_;
				}
				for (auto* ctx : snapshot) {
					for (auto* n = to_fifo<LogBlock*>(ctx->full.drain()); n;) {
						auto* node = n;
						n = n->next;
						out_file->append(node->value->data, node->value->len);
						delete node;
					}

					std::lock_guard<std::mutex> lock(ctx->cur_mtx);
					if (!ctx->cur->empty()) {
						out_file->append(ctx->cur->data, ctx->cur->len);
					}
				}
				out_file->flush();
			}
			catch (...) {
			}
		}
	}
} // namespace elog::details