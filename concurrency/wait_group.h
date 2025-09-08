// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_WAIT_GROUP_H_
#define DDV_CONCURRENCY_WAIT_GROUP_H_ 1

#include <atomic>
#include <cstdint>

#include "concurrency/one_shot_event.h"

#include "util/debug.h"
#include "util/macro.h"

namespace concurrency {

// Synchronization primitive for waiting for the completion of tasks,
// which are expressed as a 64-bit counter
// Formally, the wait ends when the counter drops to zero
//
// The add call should happens before the corresponding done call, otherwise
// the behavior is undefined
//
// The add calls should happens before the wait calls
//
// WaitGroup can be reused. In this case, all current wait calls should
// end (happens) before subsequent add calls
class WaitGroup {
private:
	using counter_t = ::std::uint64_t;

	::std::atomic<counter_t> count_ = 0;
	OneShotEvent counter_is_zero_;

public:
	~WaitGroup() = default;

	WaitGroup(WaitGroup const &) = delete;
	void operator= (WaitGroup const &) = delete;

	WaitGroup(WaitGroup &&) = delete;
	void operator= (WaitGroup &&) = delete;

public:
	WaitGroup() = default;

	// increases the counter by delta
	void add(counter_t const delta = 1) noexcept
	{
		[[maybe_unused]] auto const count
			= count_.fetch_add(delta, ::std::memory_order_relaxed);

		UTILS_ASSERT(
			count < count + delta,
			"increment must be non-zero and not overflow the counter"
		);
	}

	// reduces the counter by delta, performs synchronization, and
	// if the counter drops to zero, unblocks all threads currently waiting for
	void done(counter_t const delta = 1)
		noexcept (noexcept(counter_is_zero_.notify()))
	{
		auto const count = count_.fetch_sub(delta, ::std::memory_order_release);

		UTILS_ASSERT(
			count - delta < count,
			"decrement must be non-zero and not underflow the counter"
		);

		if (count == delta) {
			counter_is_zero_.notify();
		}
	}

	// blocks the current thread until the counter drops to zero, and
	// synchronizes with threads that performed synchronization when calling done
	void wait()
		noexcept (noexcept(counter_is_zero_.wait()))
	{
		counter_is_zero_.wait();

		// synchronization
		UTILS_IGNORE(count_.load(::std::memory_order_acquire));
	}

	// in case of instance reuse
	void clear() noexcept
	{
		counter_is_zero_.reset();
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_WAIT_GROUP_H_ */
