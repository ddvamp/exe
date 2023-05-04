// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_QUEUE_SPINLOCK_H_
#define DDV_CONCURRENCY_QUEUE_SPINLOCK_H_ 1

#include <atomic>

#include "utils/concurrency/relax.h"
#include "utils/intrusive/forward_list.h"
#include "utils/utility.h"

namespace concurrency {

class QueueSpinlock {
private:
	struct Node : ::utils::intrusive_concurrent_forward_list_node<Node> {
		::std::atomic_bool free_;
	};

	Node dummy_{nullptr, true};
	Node *head_ = &dummy_;
	::std::atomic<Node *> tail_ = &dummy_;

public:
	// for case of high contention
	class Guard {
	private:
		QueueSpinlock &lock_;
		Node node_;
		bool fast_;

	public:
		explicit Guard(QueueSpinlock &lock) noexcept
			: lock_(lock)
			, fast_(lock.try_lock())
		{
			if (!fast_) [[likely]] {
				lock.lockInit(node_);
			}
		}

		~Guard()
		{
			lock_.unlockFini(node_, fast_);
		}

		Guard(Guard const &) = delete;
		void operator= (Guard const &) = delete;

		Guard(Guard &&) = delete;
		void operator= (Guard &&) = delete;
	};

public:
	[[nodiscard]] bool try_lock() noexcept
	{
		return dummy_.free_.load(::std::memory_order_relaxed) &&
			dummy_.free_.compare_exchange_weak(
				::utils::temporary(true),
				false,
				::std::memory_order_acquire,
				::std::memory_order_relaxed
			);
	}

	void lock() noexcept
	{
		if (!try_lock()) [[unlikely]] {
			auto node = Node{};

			lockInit(node);
			lockFini(node);
		}
	}

	void unlock() noexcept
	{
		head_->free_.store(true, ::std::memory_order_release);
	}

private:
	void lockInit(Node &node) noexcept
	{
		auto const prev = tail_.exchange(&node, ::std::memory_order_acq_rel);

		if (prev == &dummy_) [[unlikely]] {
			while (!try_lock()) {
				::utils::thread_relax();
			}
		} else {
			prev->next_.store(&node, ::std::memory_order_release);

			while (!node.free_.load(::std::memory_order_acquire)) {
				::utils::thread_relax();
			}
		}
	}

	void lockFini(Node &node) noexcept
	{
		auto next = static_cast<Node *>(nullptr);

		if (
			!(next = node.next_.load(::std::memory_order_acquire)) &&

			!tail_.compare_exchange_strong(
				::utils::temporary(&node),
				next = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			)
		) [[unlikely]] {
			while (!(next = node.next_.load(::std::memory_order_acquire))) {
				::utils::thread_relax();
			}
		}

		head_ = next;
	}

	void unlockFini(Node &node, bool fast) noexcept
	{
		if (!fast) [[likely]] {
			lockFini(node);
		}
		unlock();
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_QUEUE_SPINLOCK_H_ */
