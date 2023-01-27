#ifndef DDV_CONCURRENCY_WAIT_GROUP_H_
#define DDV_CONCURRENCY_WAIT_GROUP_H_ 1

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <new>

#include "utils/debug.h"
#include "utils/macro.h"

namespace concurrency {

// Synchronization primitive for waiting for the completion of tasks,
// which are expressed as a 64-bit counter
// Formally, the wait ends when the counter reaches zero
// 
// The add call should happens before the corresponding done call
// (otherwise, we get a negative counter)
// 
// The add calls should happens before the wait calls
//
// WaitGroup can be reused. In this case, all current wait calls should
// end (happens) before subsequent add calls
class WaitGroup {
private:
	alignas(::std::hardware_destructive_interference_size)
		::std::atomic_uint64_t count_ = 0;

	alignas(::std::hardware_destructive_interference_size)
		bool there_are_waiters_ = false;

	::std::mutex m_; // protects the condvar
	::std::condition_variable counter_is_zero_;

public:
	~WaitGroup() = default;

	WaitGroup(WaitGroup const &) = delete;
	void operator= (WaitGroup const &) = delete;

	WaitGroup(WaitGroup &&) = delete;
	void operator= (WaitGroup &&) = delete;

public:
	WaitGroup() = default;

	// increases the counter
	void add(::std::uint64_t delta = 1) noexcept
	{
		[[maybe_unused]] auto count
			= count_.fetch_add(delta, ::std::memory_order_relaxed);

		UTILS_ASSERT(
			count < count + delta,
			"increment must be non-zero and not overflow the counter"
		);
	}

	// reduces the counter, releases data,
	// and if the counter has reached zero, notifies
	void done(::std::uint64_t delta = 1)
	{
		auto count = count_.fetch_sub(delta, ::std::memory_order_release);

		UTILS_ASSERT(
			count - delta < count,
			"decrement must be non-zero and not underflow the counter"
		);

		if (count == delta) {
			if (::std::lock_guard lock{m_}; there_are_waiters_) {
				there_are_waiters_ = false;
			} else {
				return;
			}

			counter_is_zero_.notify_all();
		}
	}

	// waits until the counter drops to zero and acquire data
	void wait()
	{
		::std::unique_lock lock{m_};

		while (count_.load(::std::memory_order_acquire) != 0) {
			there_are_waiters_ = true;
			counter_is_zero_.wait(lock);
		}
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_WAIT_GROUP_H_ */
