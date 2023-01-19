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
			dummy_.next_.compare_exchange_strong(
				::utils::temporary(&dummy_),
				nullptr,
				::std::memory_order_acquire,
				::std::memory_order_relaxed
			);
	}

	void lock() noexcept
	{
		if (!try_lock()) [[unlikely]] {
			auto awaiter = LockAwaiter{this};
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

	// Puts fiber node in queue
	// If lock has passed to current THREAD in the process, returns false
	bool giveNodeOrGetControl(Node *node) noexcept
	{
		return !tail_.exchange(node, ::std::memory_order_acq_rel)
			->	next_.exchange(node, ::std::memory_order_acq_rel);
	}

	// Gets fiber node from queue (following known next owner or dummy)
	// Otherwise, if there isn't one yet, passes control and returns nullptr
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
	FiberHandle getFiberFrom(Node *next) noexcept
	{
		// cleanup
		dummy_.next_.store(nullptr, ::std::memory_order_relaxed);

		head_ = next;

		return getFiberFromKnownNextOwner();
	}

	bool isNextOwnerKnown() const noexcept
	{
		return head_ != &dummy_;
	}

	FiberHandle lockImpl(FiberInfo *node) noexcept
	{
		if (giveNodeOrGetControl(node)) [[likely]] {
			return FiberHandle::invalid();
		}

		if (isNextOwnerKnown()) [[unlikely]] {
			replaceKnownNextOwner(node).schedule();

			return FiberHandle::invalid();
		}

		return getFiberFrom(node); // ~ getNodeOrGiveControl
	}

	FiberHandle unlockImpl() noexcept
	{
		if (isNextOwnerKnown()) [[unlikely]] {
			return getFiberFromKnownNextOwner(); // ~ getNodeOrGiveControl
		}

		if (auto node = getNodeOrGiveControl(); node) [[unlikely]] {
			return getFiberFrom(node); // ~ getNodeOrGiveControl
		}

		return FiberHandle::invalid();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_MUTEX_H_ */
