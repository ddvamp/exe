#ifndef DDV_EXE_FIBERS_SYNC_MUTEX_H_
#define DDV_EXE_FIBERS_SYNC_MUTEX_H_ 1

#include <atomic>
#include <mutex>

#include "exe/fibers/api.h"
#include "exe/fibers/core/awaiter.h"
#include "exe/fibers/core/handle.h"

#include "utils/intrusive/forward_list.h"
#include "utils/utility.h"

namespace exe::fibers {

class Mutex {
private:
	struct FiberInfo : ::utils::intrusive_concurrent_forward_list_node<> {
		FiberHandle handle_;
	};

	class LockAwaiter : public IAwaiter {
	private:
		FiberInfo info_;
		Mutex *m_;

	public:
		explicit LockAwaiter(Mutex *m) noexcept
			: m_(m)
		{}

		void awaitSuspend(FiberHandle &&) noexcept override
		{
			// nothing
		}

		[[nodiscard]] FiberHandle awaitSymmetricSuspend(
			FiberHandle &&current) noexcept override
		{
			info_.handle_ = ::std::move(current);

			return m_->lockImpl(&info_);
		}
	};

	using Node = FiberInfo::Node;

	Node dummy_;
	Node *head_ = &dummy_;
	::std::atomic<Node *> tail_ = nullptr;

public:
	~Mutex() = default;

	Mutex(Mutex const &) = delete;
	void operator= (Mutex const &) = delete;

	Mutex(Mutex &&) = delete;
	void operator= (Mutex &&) = delete;

public:
	Mutex() noexcept = default;

	[[nodiscard]] bool try_lock() noexcept
	{
		return !tail_.load(::std::memory_order_relaxed) &&
			tail_.compare_exchange_strong(
				::utils::temporary(static_cast<Node *>(nullptr)),
				&dummy_,
				::std::memory_order_acquire,
				::std::memory_order_relaxed
			);
	}

	void lock() noexcept
	{
		if (!try_lock()) {
			LockAwaiter awaiter{this};
			self::suspend(awaiter);
		}
	}

	void unlock() noexcept
	{
		auto next = unlockImpl();

		if (next.isValid()) {
			::std::move(next).schedule();
		}
	}

private:
	[[nodiscard]] FiberHandle exchangeHeadFor(Node *next) noexcept
	{
		auto curr = ::std::exchange(head_, next);

		[[assume(curr)]];
		auto info = static_cast<FiberInfo *>(curr);

		return ::std::move(info->handle_);
	}

	[[nodiscard]] FiberHandle takeNextFiber(Node *next) noexcept
	{
		head_ = next;

		if (
			(next = head_->next_.load(::std::memory_order_acquire)) ||
			
			tail_.compare_exchange_strong(
				::utils::temporary(::utils::decay_copy(head_)),
				next = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			) ||
			
			(next = head_->next_.load(::std::memory_order_acquire)) ||

			!head_->next_.compare_exchange_strong(
				next,
				head_,
				::std::memory_order_release,
				::std::memory_order_acquire
			)
		) {
			return exchangeHeadFor(next);
		}

		return FiberHandle::invalid();
	}

	[[nodiscard]] FiberHandle lockImpl(FiberInfo *fiber) noexcept
	{
		auto prev = tail_.exchange(fiber, ::std::memory_order_acq_rel);

		if (!prev) [[unlikely]] {
			return takeNextFiber(fiber);
		}
			
		if (
			prev->next_.compare_exchange_strong(
				::utils::temporary(static_cast<Node *>(nullptr)),
				fiber,
				::std::memory_order_release,
				::std::memory_order_acquire
			)
		) [[likely]] {
			return FiberHandle::invalid();
		}

		if (prev == &dummy_) [[likely]] {
			prev->next_.store(nullptr, ::std::memory_order_relaxed);
			return takeNextFiber(fiber);
		}

		exchangeHeadFor(fiber).schedule();

		return FiberHandle::invalid();
	}

	[[nodiscard]] FiberHandle unlockImpl() noexcept
	{
		auto next = head_;

		if (
			next != &dummy_ ||

			(
				(next = head_->next_.load(::std::memory_order_acquire)) ||

				!tail_.compare_exchange_strong(
					::utils::temporary(::utils::decay_copy(head_)),
					nullptr,
					::std::memory_order_release,
					::std::memory_order_relaxed
				) &&

				(
					(next = head_->next_.load(::std::memory_order_acquire)) ||

					!head_->next_.compare_exchange_strong(
						next,
						head_,
						::std::memory_order_release,
						::std::memory_order_acquire
					)
				)
			) &&
			(head_->next_.store(nullptr, ::std::memory_order_relaxed), true)
		) {
			return takeNextFiber(next);
		}

		return FiberHandle::invalid();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_MUTEX_H_ */
