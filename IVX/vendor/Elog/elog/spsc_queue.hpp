#pragma once

#include <atomic>
#include <cassert>
#include <utility>

namespace elog::details {
	// SPSC 
	template <typename T>
	class SpscQueue {
	public:
		struct Node {
			Node(T&& v, Node* n) : value(std::move(v)), next(n) {
			}

			T value;
			Node* next;
		};

		// ---- Producer(single thread) ----
		void push(T v) {
			auto* n = new Node(std::move(v), pending_);
			if (!pending_tail_) {
				pending_tail_ = n;
			}
			pending_ = n;
		}

		void flush() {
			if (!pending_) {
				return;
			}
			pending_tail_->next = head_.load(std::memory_order_acquire);
			head_.store(pending_, std::memory_order_release);
			pending_ = nullptr;
			pending_tail_ = nullptr;
		}

		// ---- Consumer(single thread) ----
		Node* drain() {
			return head_.exchange(nullptr, std::memory_order_acquire);
		}

	private:
		std::atomic<Node*> head_{ nullptr };
		Node* pending_{ nullptr };
		Node* pending_tail_{ nullptr };
	};

	// reverse
	template <typename T>
	typename SpscQueue<T>::Node* to_fifo(typename SpscQueue<T>::Node* list) {
		typename SpscQueue<T>::Node* fifo = nullptr;
		while (list) {
			auto* n = list;
			list = list->next;
			n->next = fifo;
			fifo = n;
		}
		return fifo;
	}
} // namespace elog::details