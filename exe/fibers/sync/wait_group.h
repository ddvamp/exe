#ifndef DDV_EXE_FIBERS_SYNC_WAIT_GROUP_H_
#define DDV_EXE_FIBERS_SYNC_WAIT_GROUP_H_ 1

#include <atomic>
#include <cstdint>
#include <new>

#include "exe/fibers/sync/condvar.h"
#include "exe/fibers/sync/mutex.h"

#include "utils/debug.h"
#include "utils/macro.h"

namespace exe::fibers {

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
		Mutex m_; // protects the condvar
	CondVar counter_is_zero_{m_};

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
	void done(::std::uint64_t delta = 1) noexcept
	{
		auto count = count_.fetch_sub(delta, ::std::memory_order_release);

		UTILS_ASSERT(
			count - delta < count,
			"decrement must be non-zero and not underflow the counter"
		);

		if (count == delta) {
			::std::lock_guard lock{m_};

			counter_is_zero_.notify_all();
		}
	}

	// waits until the counter drops to zero and acquire data
	void wait() noexcept
	{
		::std::lock_guard lock{m_};

		while (count_.load(::std::memory_order_acquire) != 0) {
			counter_is_zero_.wait();
		}
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_WAIT_GROUP_H_ */
