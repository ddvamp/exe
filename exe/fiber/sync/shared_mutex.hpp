// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_SYNC_SHARED_MUTEX_H_
#define DDV_EXE_FIBER_SYNC_SHARED_MUTEX_H_ 1

#include <atomic>
#include <cstdint>
#include <shared_mutex>
#include <utility>

#include "exe/fiber/api.hpp"
#include "exe/fiber/core/awaiter.hpp"
#include "exe/fiber/core/handle.hpp"
#include "exe/fiber/sync/mutex.hpp"

#include "util/macro.hpp"

namespace exe::fiber {

class SharedMutex {
	class LockAwaiter : public IAwaiter {
	private:
		SharedMutex *m_;

	public:
		explicit LockAwaiter(SharedMutex* m) noexcept
			: m_(m)
		{}

		void awaitSuspend(FiberHandle &&) noexcept override
		{
			// nothing
		}

		[[nodiscard]] FiberHandle awaitSymmetricSuspend(
			FiberHandle &&current) noexcept override
		{
			return m_->setWriter(::std::move(current));
		}
	};

	Mutex mutex_;
	::std::uint64_t in_ = 0;
	::std::atomic_uint64_t out_ = 0;
	FiberHandle writer_;

public:
	~SharedMutex() = default;

	SharedMutex(SharedMutex const &) = delete;
	void operator= (SharedMutex const &) = delete;

	SharedMutex(SharedMutex &&) = delete;
	void operator= (SharedMutex &&) = delete;

public:
	SharedMutex() = default;

	[[nodiscard]] bool try_lock() noexcept
	{
		if (!mutex_.try_lock()) {
			return false;
		}

		if (in_ != out_.load(::std::memory_order_relaxed)) {
			mutex_.unlock();
			return false;
		}

		return true;
	}

	void lock() noexcept
	{
		mutex_.lock();

		if (in_ == out_.load(::std::memory_order_relaxed)) {
			return;
		}

		auto awaiter = LockAwaiter(this);
		self::suspend(awaiter);

		in_ = 0;
		out_.store(0, ::std::memory_order_relaxed);
	}

	void unlock() noexcept
	{
		mutex_.unlock();
	}

	[[nodiscard]] bool try_lock_shared() noexcept
	{
		if (!mutex_.try_lock()) {
			return false;
		}

		UTIL_IGNORE(in_ += 0b10);

		mutex_.unlock();

		return true;
	}

	void lock_shared() noexcept
	{
		auto lock = ::std::lock_guard(mutex_);

		UTIL_IGNORE(in_ += 0b10);
	}

	void unlock_shared() noexcept
	{
		auto const state = out_.fetch_add(0b10, ::std::memory_order_relaxed);

		if (state == (0b01 - 0b10ull)) {
			// synchronization of writer_
			UTIL_IGNORE(out_.load(::std::memory_order_acquire));

			::std::move(writer_).schedule();
		}
	}

private:
	[[nodiscard]] FiberHandle setWriter(FiberHandle &&writer) noexcept
	{
		writer_ = ::std::move(writer);

		// synchronization of writer_
		auto out = out_.fetch_add(0b01 - in_, ::std::memory_order_release);

		return out == in_ ? ::std::move(writer_) : FiberHandle::invalid();
	}
};

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_SYNC_SHARED_MUTEX_H_ */
