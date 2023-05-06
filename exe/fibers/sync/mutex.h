// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

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

class CondVar;

class Mutex {
private:
	friend class CondVar;

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

	Node dummy_{&dummy_};
	Node *head_ = &dummy_; // known next owner or dummy
	::std::atomic<Node *> tail_ = &dummy_; // last added or dummy

public:
	~Mutex() = default;

	Mutex(Mutex const &) = delete;
	void operator= (Mutex const &) = delete;

	Mutex(Mutex &&) = delete;
	void operator= (Mutex &&) = delete;

public:
	Mutex() = default;

	[[nodiscard]] bool try_lock() noexcept
	{
		return dummy_.next_.load(::std::memory_order_relaxed) == &dummy_ &&
			dummy_.next_.compare_exchange_weak(
				::utils::temporary(&dummy_),
				nullptr,
				::std::memory_order_acquire,
				::std::memory_order_relaxed
			);
	}

	void lock() noexcept
	{
		if (!try_lock()) [[unlikely]] {
			auto awaiter = LockAwaiter(this);
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
	// sets new known next owner and returns FiberHandle of previous one
	FiberHandle replaceKnownNextOwner(Node *next) noexcept
	{
		auto curr = ::std::exchange(head_, next);

		[[assume(curr)]];
		auto info = static_cast<FiberInfo *>(curr);

		return ::std::move(info->handle_);
	}

	// puts fiber in the waiting queue (in any case) and returns true only if
	// no ownership was obtained by the current THREAD at the same time
	bool giveNodeOrGetControl(Node *node) noexcept
	{
		return !tail_.exchange(node, ::std::memory_order_acq_rel)
			->	next_.exchange(node, ::std::memory_order_acq_rel);
	}

	// Takes fiber from the waiting queue (following known next owner or dummy)
	// and return it
	// Otherwise, if there isn't one yet, gives ownership and returns nullptr
	Node *getNodeOrGiveControl() const noexcept
	{
		// be optimistic
		auto node = head_->next_.load(::std::memory_order_acquire);
			
		if (!node) {
			head_->next_.compare_exchange_strong(
				node,
				head_,
				::std::memory_order_release,
				::std::memory_order_acquire
			);
		}

		return node;
	}
	
	// Gets exclusive ownership of known next owner and returns his FiberHandle
	// Otherwise, passes control and returns invalid FiberHandle
	FiberHandle getFiberFromKnownNextOwner() noexcept
	{
		if (auto node = head_->next_.load(::std::memory_order_acquire);
			// be optimistic
			node ||
			
			tail_.compare_exchange_strong(
				::utils::temporary(::utils::decay_copy(head_)),
				node = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			) ||
			
			(node = getNodeOrGiveControl())
		) {
			return replaceKnownNextOwner(node);
		}

		return FiberHandle::invalid();
	}

	// sets known next owner instead of dummy and tries to get his FiberHandle
	// (tries to exclusively take ownership of it)
	FiberHandle getFiberFrom(Node *next) noexcept
	{
		// cleanup
		dummy_.next_.store(nullptr, ::std::memory_order_relaxed);

		head_ = next;

		return getFiberFromKnownNextOwner();
	}

	// returns true if it is known which fiber is the next owner
	bool isNextOwnerKnown() const noexcept
	{
		return head_ != &dummy_;
	}

	FiberHandle lockImpl(FiberInfo *node) noexcept
	{
		if (giveNodeOrGetControl(node)) [[likely]] {
			// successfully put the current fiber in the waiting queue
			return FiberHandle::invalid();
		}
		// Getting control over the mutex state
		// The current fiber has become shared (by waiting queue)

		if (isNextOwnerKnown()) [[unlikely]] {
			// There is another fiber in front of the current one
			// The mutex exclusively owns it, so it can be scheduled
			// The current fiber becomes the next one for scheduling (owner)
			replaceKnownNextOwner(node).schedule();
			return FiberHandle::invalid();
		}
		// The current fiber is next one for scheduling, but
		// it still doesn't own the lock because it has just been shared
		// It is necessary to prevent sharing
		return getFiberFrom(node); // ~ getNodeOrGiveControl
	}

	FiberHandle unlockImpl() noexcept
	{
		if (isNextOwnerKnown()) [[unlikely]] {
			// there is the fiber for scheduling, but it may be shared, so
			// try to exclusively take ownership of it, and
			// if successful, schedule it
			return getFiberFromKnownNextOwner(); // ~ getNodeOrGiveControl
		}
		// the next fiber for scheduling is unknown, so
		// try to get fiber from the waiting queue

		if (auto node = getNodeOrGiveControl(); node) [[unlikely]] {
			// if the next fiber for scheduling has been received, record it
			// as the next owner and follow the steps above
			return getFiberFrom(node); // ~ getNodeOrGiveControl
		}
		// the next fiber for scheduling was not received and control was given

		return FiberHandle::invalid();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_MUTEX_H_ */
