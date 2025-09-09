// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_SYNC_MUTEX_H_
#define DDV_EXE_FIBER_SYNC_MUTEX_H_ 1

#include <atomic>
#include <mutex>
#include <utility>

#include "exe/fiber/api.h"
#include "exe/fiber/core/awaiter.h"
#include "exe/fiber/core/handle.h"

#include "util/debug.h"
#include "concurrency/intrusive/forward_list.h"

namespace exe::fiber {

class CondVar;

class Mutex {
private:
	friend class CondVar;

	struct FiberInfo : ::util::intrusive_concurrent_forward_list_node<> {
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

#ifndef UTIL_DISABLE_ASSERT
	::std::atomic<FiberId> owner_ = kInvalidFiberId;

	FiberId getOwner() const noexcept
	{
		return owner_.load(::std::memory_order_relaxed);
	}

	void setOwner(FiberId id) noexcept
	{
		owner_.store(id, ::std::memory_order_relaxed);
	}

	void checkOwner() const noexcept
	{
		UTIL_CHECK(
			getOwner() == self::getId(),
			"the mutex is not locked"
		);
	}

	void checkRecursive() const noexcept
	{
		UTIL_CHECK(
			getOwner() != self::getId(),
			"attempt to recursively lock a mutex"
		);
	}

	void checkLock() noexcept
	{
		UTIL_CHECK(
			getOwner() == kInvalidFiberId,
			"attempt to lock a already locked mutex"
		);
		setOwner(self::getId());
	}

	void checkUnlock() noexcept
	{
		UTIL_CHECK(
			getOwner() == self::getId(),
			"attempt to unlock a mutex without ownership"
		);
		setOwner(kInvalidFiberId);
	}
#endif

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
		UTIL_RUN(checkRecursive);

		auto expected = &dummy_;

		return dummy_.next_.compare_exchange_weak(
			expected,
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

		UTIL_RUN(checkLock);
	}

	void unlock() noexcept
	{
		UTIL_RUN(checkUnlock);

		auto next = unlockImpl();

		if (next.isValid()) {
			::std::move(next).schedule();
		}
	}

private:
	FiberHandle lockImpl(FiberInfo *node) noexcept
	{
		auto owner = putFiber(node);

		if (!owner) [[likely]] {
			return FiberHandle::invalid();
		}

		if (isActualFiber(owner)) [[unlikely]] {
			setNext(node);
			extractFiber(owner).schedule();
			return FiberHandle::invalid();
		}

		dummy_.link(nullptr);
		return takeLastFiber(node); // ~ putFiber
	}

	FiberHandle unlockImpl() noexcept
	{
		auto owner = getNext();

		if (isActualFiber(owner)) [[unlikely]] {
			return takeFiber(owner); // ~ putFiber
		}

		if ((owner = tryTakeNextFiber(owner))) [[unlikely]] {
			dummy_.link(nullptr);
			return takeFiber(owner); // ~ putFiber
		}

		return FiberHandle::invalid();
	}

	FiberHandle takeFiber(Node *owner) noexcept
	{
		if (auto next = loadNext(owner)) {
			setNext(next);
			return extractFiber(owner);
		}

		return takeLastFiber(owner);
	}

	FiberHandle takeLastFiber(Node *owner) noexcept
	{
		auto next = &dummy_;

		if (tryPutDummy(owner) ||

			(next = tryTakeNextFiber(owner))
		) {
			setNext(next);
			return extractFiber(owner);
		}

		return FiberHandle::invalid();
	}

	bool isActualFiber(Node *node) const noexcept
	{
		return node != &dummy_;
	}

	Node *getNext() const noexcept
	{
		return head_;
	}

	void setNext(Node *node) noexcept
	{
		head_ = node;
	}

	static FiberHandle extractFiber(Node *node) noexcept
	{
		[[assume(node)]];
		return ::std::move(*static_cast<FiberInfo *>(node)).handle_;
	}

	static Node *loadNext(Node *node) noexcept
	{
		return node->next_.load(::std::memory_order_acquire);
	}

	Node *putFiber(Node *node) noexcept
	{
		return
			tail_.exchange(node, ::std::memory_order_acq_rel)->
			next_.exchange(node, ::std::memory_order_acq_rel);
	}

	bool tryPutDummy(Node *expected) noexcept
	{
		return tail_.compare_exchange_strong(
			expected,
			&dummy_,
			::std::memory_order_release,
			::std::memory_order_relaxed
		);
	}

	Node *tryTakeNextFiber(Node *node) noexcept
	{
		setNext(node);

		auto next = static_cast<Node *>(nullptr);

		node->next_.compare_exchange_strong(
			next,
			node,
			::std::memory_order_release,
			::std::memory_order_acquire
		);

		return next;
	}
};

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_SYNC_MUTEX_H_ */
