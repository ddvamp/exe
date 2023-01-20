#ifndef DDV_CONCURRENCY_QUEUE_SPINLOCK_H_
#define DDV_CONCURRENCY_QUEUE_SPINLOCK_H_ 1

#include <atomic>

#include "utils/intrusive/forward_list.h"
#include "utils/utility.h";

namespace concurrency {

class QueueSpinlock {
private:
	struct Node : ::utils::intrusive_concurrent_forward_list_node<Node> {
		::std::atomic_bool free_;
	};

	Node dummy_{nullptr, true};
	Node *head_;
	::std::atomic<Node *> tail_ = &dummy_;

public:
	void lock() noexcept
	{
		auto node = Node{};

		if (
			auto prev = tail_.exchange(&node, ::std::memory_order_acq_rel);
			prev == &dummy_
		) [[likely]] {
			while (!dummy_.free_.load(::std::memory_order_acquire));

			dummy_.free_.store(false, ::std::memory_order_relaxed);
		} else {
			prev->next_.store(&node, ::std::memory_order_release);

			while (!node.free_.load(::std::memory_order_acquire));
		}

		if (
			!(head_ = node.next_.load(::std::memory_order_acquire)) &&

			!tail_.compare_exchange_strong(
				::utils::temporary(&node),
				head_ = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			)
		) [[unlikely]] {
			while (!(head_ = node.next_.load(::std::memory_order_acquire)));
		}
	}

	void unlock() noexcept
	{
		head_->free_.store(true, ::std::memory_order_release);
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_QUEUE_SPINLOCK_H_ */
