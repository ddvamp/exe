//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ 1

#include <cstdint>
#include <utility>

#include "exe/fiber/api.hpp"
#include "exe/fiber/core/awaiter.hpp"
#include "exe/fiber/sync/mutex.hpp"

namespace exe::fiber {

class CondVar {
private:
	using FiberInfo = Mutex::FiberInfo;

	using Node = Mutex::Node;

	struct Waiters {
		Node *head_ = nullptr;
		Node *tail_ = nullptr;
		::std::uint64_t count_ = 0;
	};

	class WaitAwaiter : public IAwaiter {
	private:
		FiberInfo info_;
		CondVar *cv_;

	public:
		explicit WaitAwaiter(CondVar *cv) noexcept
			: cv_(cv)
		{}

		void awaitSuspend(FiberHandle &&) noexcept override
		{
			// nothing
		}

		[[nodiscard]] FiberHandle awaitSymmetricSuspend(
			FiberHandle &&current) noexcept override
		{
			info_.handle_ = ::std::move(current);

			cv_->waitImpl(&info_);

			return FiberHandle::invalid();
		}
	};

	Mutex &mutex_;
	Waiters waiters_;

public:
	~CondVar() = default;

	CondVar(CondVar const &) = delete;
	void operator= (CondVar const &) = delete;

	CondVar(CondVar &&) = delete;
	void operator= (CondVar &&) = delete;

public:
	explicit CondVar(Mutex &mutex) noexcept
		: mutex_(mutex)
	{}

	/* ATTENTION: All methods below require a locked mutex_! */

	void wait() noexcept
	{
		UTIL_RUN(mutex_.checkOwner);

		WaitAwaiter awaiter{this};
		self::suspend(awaiter);
	}

	template <typename Predicate>
	void wait(Predicate &&stop_waiting) noexcept
	{
		UTIL_RUN(mutex_.checkOwner);

		while (!stop_waiting()) {
			wait();
		}
	}

	::std::uint64_t notify_one() noexcept
	{
		return notifyImpl(1);
	}

	::std::uint64_t notify_all() noexcept
	{
		return notifyImpl(0ull - 1);
	}

	::std::uint64_t notify_n(::std::uint64_t count) noexcept
	{
		return count != 0 ? notifyImpl(count) : 0;
	}

private:
	void addWaiter(FiberInfo *waiter) noexcept
	{
		if (waiters_.count_ != 0) {
			waiters_.tail_->next_.store(waiter, ::std::memory_order_relaxed);
		} else {
			waiters_.head_ = waiter;
		}

		waiters_.tail_ = waiter;

		++waiters_.count_;
	}

	void waitImpl(FiberInfo *info) noexcept
	{
		addWaiter(info);

		mutex_.unlock();
	}

	[[nodiscard]] Waiters takeWaiters(::std::uint64_t count) noexcept
	{
		if (waiters_.count_ <= count) {
			return ::std::exchange(waiters_, {});
		}

		auto head = waiters_.head_;
		auto tail = waiters_.head_;
		auto cnt = count;

		while (--count != 0) {
			tail = tail->next_.load(::std::memory_order_relaxed);
		}

		waiters_.head_ = tail->next_.load(::std::memory_order_relaxed);
		waiters_.count_ -= cnt;

		tail->next_.store(nullptr, ::std::memory_order_relaxed);

		return {head, tail, cnt};
	}

	[[nodiscard]] ::std::uint64_t notifyImpl(::std::uint64_t count) noexcept
	{
		UTIL_RUN(mutex_.checkOwner);

		if (waiters_.count_ == 0) {
			return 0;
		}

		auto awakeneds = takeWaiters(count);

		auto node =
			mutex_.tail_.exchange(awakeneds.tail_, ::std::memory_order_acq_rel);

		node->next_.store(awakeneds.head_, ::std::memory_order_relaxed);

		return awakeneds.count_;
	}
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ */
