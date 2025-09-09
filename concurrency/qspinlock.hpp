//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ 1

#include <atomic>
#include <mutex>

#include "pause.hpp"
#include "intrusive/forward_list.hpp"
#include "util/utility.hpp"

namespace concurrency {

class QSpinlock {
private:
	struct Node : intrusive_concurrent_forward_list_node<Node> {
		::std::atomic_bool free_;
	};

	Node dummy_{nullptr, true};
	Node *head_ = &dummy_;
	::std::atomic<Node *> tail_ = &dummy_;

public:
	// for case of high contention
	class LockToken {
	private:
		QSpinlock &lock_;
		Node node_;
		bool fast_;

	public:
		~LockToken() = default;

		LockToken(LockToken const &) = delete;
		void operator= (LockToken const &) = delete;

		LockToken(LockToken &&) = delete;
		void operator= (LockToken &&) = delete;

	public:
		explicit LockToken(QSpinlock &spinlock) noexcept
			: lock_(spinlock)
		{
			lock();
		}

		LockToken(QSpinlock &spinlock, ::std::defer_lock_t) noexcept
			: lock_(spinlock)
		{}

		LockToken(QSpinlock &spinlock, ::std::adopt_lock_t) noexcept
			: lock_(spinlock)
			, fast_(true)
		{}

	public:
		void lock() noexcept
		{
			fast_ = lock_.try_lock();

			if (!fast_) [[likely]] {
				lock_.lockInit(node_);
			}
		}

		void unlock() noexcept
		{
			lock_.unlockFini(node_, fast_);
		}
	};

	[[nodiscard]] auto get_token() noexcept
	{
		return LockToken(*this);
	}

	[[nodiscard]] auto get_deferred_token() noexcept
	{
		return LockToken(*this, ::std::defer_lock);
	}

	// precondition: locked
	[[nodiscard]] auto get_adopted_token() noexcept
	{
		return LockToken(*this, ::std::adopt_lock);
	}

	// for case of high contention
	class Guard : protected LockToken {
	public:
		explicit Guard(QSpinlock &lock) noexcept
			: LockToken(lock)
		{}

		~Guard()
		{
			unlock();
		}
	};

	[[nodiscard]] auto lock_with_guard() noexcept
	{
		return Guard(*this);
	}

public:
	[[nodiscard]] bool try_lock() noexcept
	{
		return dummy_.free_.load(::std::memory_order_relaxed) &&
			dummy_.free_.compare_exchange_weak(
				::util::temporary(true),
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
				thread_relax();
			}
		} else {
			prev->next_.store(&node, ::std::memory_order_release);

			while (!node.free_.load(::std::memory_order_acquire)) {
				thread_relax();
			}
		}
	}

	void lockFini(Node &node) noexcept
	{
		auto next = static_cast<Node *>(nullptr);

		if (
			!(next = node.next_.load(::std::memory_order_acquire)) &&

			!tail_.compare_exchange_strong(
				::util::temporary(&node),
				next = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			)
		) [[unlikely]] {
			while (!(next = node.next_.load(::std::memory_order_acquire))) {
				thread_relax();
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

#endif /* DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ */
