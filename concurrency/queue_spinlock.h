// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_QUEUE_SPINLOCK_H_
#define DDV_CONCURRENCY_QUEUE_SPINLOCK_H_ 1

#include <atomic>

#include "utils/intrusive/forward_list.h"
#include "utils/utility.h";

namespace concurrency {

namespace detail {

inline void thread_relax() noexcept
{
#if (defined(__GNUC__) || defined(__GNUG__)) &&				\
	!defined(__clang__) && !defined(__INTEL_COMPILER) &&	\
	(defined(__x86_64__) || defined(__i386__))

	__builtin_ia32_pause();

#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))

	_mm_pause();

#endif
}

} // namespace detail

class QueueSpinlock {
private:
	struct Node : ::utils::intrusive_concurrent_forward_list_node<Node> {
		::std::atomic_bool free_;
	};

	Node dummy_{nullptr, true};
	Node *head_;
	::std::atomic<Node *> tail_ = &dummy_;

public:
	// for case of high contention
	class Guard {
	private:
		QueueSpinlock &lock_;
		Node node_;

	public:
		explicit Guard(QueueSpinlock &lock) noexcept
			: lock_(lock)
		{
			lock_.lockInit(node_);
		}

		~Guard()
		{
			lock_.unlockFini(node_);
		}

		Guard(Guard const &) = delete;
		void operator= (Guard const &) = delete;

		Guard(Guard &&) = delete;
		void operator= (Guard &&) = delete;
	};

public:
	void lock() noexcept
	{
		auto node = Node{};

		lockInit(node);
		lockFini(node);
	}

	void unlock() noexcept
	{
		head_->free_.store(true, ::std::memory_order_release);
	}

private:
	void lockInit(Node &node) noexcept
	{
		auto prev = tail_.exchange(&node, ::std::memory_order_acq_rel);

		if (prev == &dummy_) [[likely]] {
			while (!dummy_.free_.load(::std::memory_order_acquire)) {
				detail::thread_relax();
			}

			dummy_.free_.store(false, ::std::memory_order_relaxed);
		} else {
			prev->next_.store(&node, ::std::memory_order_release);

			while (!node.free_.load(::std::memory_order_acquire)) {
				detail::thread_relax();
			}
		}
	}

	void lockFini(Node &node) noexcept
	{
		if (
			!(head_ = node.next_.load(::std::memory_order_acquire)) &&

			!tail_.compare_exchange_strong(
				::utils::temporary(&node),
				head_ = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			)
		) [[unlikely]] {
			while (!(head_ = node.next_.load(::std::memory_order_acquire))) {
				detail::thread_relax();
			}
		}
	}

	void unlockFini(Node &node) noexcept
	{
		lockFini(node);
		unlock();
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_QUEUE_SPINLOCK_H_ */
