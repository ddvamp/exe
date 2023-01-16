#ifndef DDV_EXE_FIBERS_SYNC_WAIT_POINT_H_
#define DDV_EXE_FIBERS_SYNC_WAIT_POINT_H_ 1

#include <atomic>
#include <cstdint>
#include <new>

#include "exe/fibers/sync/condvar.h"
#include "exe/fibers/sync/mutex.h"

#include "utils/debug.h"
#include "utils/macro.h"

namespace exe::fibers {

// Synchronization primitive for waiting for the completion of tasks,
// which are expressed as a 32-bit counter
// Formally, the wait ends when the counter reaches zero
// 
// The add call should happens before the corresponding done call
// (otherwise, we get a negative counter)
// 
// The counter increment and wait operations are independent
// (WaitPoint is not one-time),
// so you can wait without even adding new tasks. However, if WaitPoint is
// used to wait for specific tasks to complete, the add call should be
// happens before the wait call
class WaitPoint {
private:
	inline static constexpr ::std::uint64_t helping_bit = 0x8000'0000;

	// [  32bit  |   bit   |  31bit  ]
	// [ version | helping | counter ]
	alignas(::std::hardware_destructive_interference_size)
		::std::atomic_uint64_t state_ = helping_bit; // helping_bit is set

	alignas(::std::hardware_destructive_interference_size)
		Mutex m_; // protects the condvar
	CondVar counter_is_zero_{m_};

public:
	~WaitPoint() = default;

	WaitPoint(WaitPoint const &) = delete;
	void operator= (WaitPoint const &) = delete;

	WaitPoint(WaitPoint &&) = delete;
	void operator= (WaitPoint &&) = delete;

public:
	WaitPoint() = default;

	// increases the counter
	void add(::std::uint64_t delta = 1) noexcept
	{
		// increasing logically
		[[maybe_unused]] auto state
			= state_.fetch_sub(delta, ::std::memory_order_relaxed);

		UTILS_ASSERT(
			delta - 1 < getCounter(state - 1),
			"increment must be non-zero and "
			"not overflow the 31-bit counter"
		);
	}

	// reduces the counter, releases a data and,
	// if the counter has reached zero, tries to notify
	void done(::std::uint64_t delta = 1) noexcept
	{
		// reducing logically
		auto state = state_.fetch_add(delta, ::std::memory_order_release);

		UTILS_ASSERT(
			delta - 1 < getCounter(0 - state),
			"decrement must be non-zero and "
			"not underflow the 31-bit counter"
		);

		if (isNotifyNeeded(state + delta)) {
			::std::lock_guard lock{m_};

			counter_is_zero_.notify_all();
		}
	}

	// waits until the counter drops to zero, and then acquire a data
	void wait() noexcept
	{
		auto state = state_.load(::std::memory_order_relaxed);

		// while helping bit isn't set
		while (
			(state & helping_bit) == 0 &&

			!state_.compare_exchange_weak(
				state,
				state | helping_bit,
				::std::memory_order_relaxed
			)
		);

		if (getCounter(state) != 0) [[likely]] {
			::std::lock_guard lock{m_};

			while (isWaitNeeded(state | helping_bit)) {
				counter_is_zero_.wait();
			}
		}

		// synchronize
		UTILS_IGNORE(state_.load(::std::memory_order_acquire));
	}

private:
	// State is transient if the counter has been lowered to zero
	// and the helping_bit isn't set (bc of the carry bit)
	//
	// In this case, progress is needed
	static bool isTransientState(::std::uint64_t state) noexcept
	{
		return static_cast<::std::uint32_t>(state) == 0;
	}

	// progress has occurred if the version has not rolled back
	bool tryToProgress(::std::uint64_t state) noexcept
	{
		auto const version = static_cast<::std::uint32_t>(state >> 32) - 1;

		UTILS_IGNORE(state_.compare_exchange_strong(
			state,
			state | helping_bit,
			::std::memory_order_relaxed
		));

		return static_cast<::std::uint32_t>(state >> 32) != version;
	}

	bool isNotifyNeeded(::std::uint64_t state) noexcept
	{
		return isTransientState(state) && tryToProgress(state);
	}

	// We need to keep waiting if for state_
	// version corresponds, helping bit is set and counter is non-zero,
	// or version is one more, helping bit isn't set, and counter is zero
	// 
	// It is checked in one comparison by bit magic/manipulation
	bool isWaitNeeded(::std::uint64_t const waiting_state) const noexcept
	{
		constexpr auto counter_cleaning_mask = 0xFFFF'FFFF'8000'0000ull;

		auto state = state_.load(::std::memory_order_relaxed);

		return ((state - 1) & counter_cleaning_mask) ==
			(waiting_state & counter_cleaning_mask);
	}

	static ::std::uint64_t getCounter(::std::uint64_t state) noexcept
	{
		constexpr auto counter_mask = 0x7FFF'FFFFull;

		return state & counter_mask;
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_WAIT_POINT_H_ */
