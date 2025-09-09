// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_WAIT_POINT_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_WAIT_POINT_HPP_INCLUDED_ 1

#include <atomic>
#include <cstdint>
#include <new>

#include "exe/fiber/sync/condvar.hpp"
#include "exe/fiber/sync/mutex.hpp"

#include "util/debug.hpp"
#include "util/macro.hpp"

namespace exe::fiber {

// Synchronization primitive for waiting for the completion of tasks,
// which are expressed as a 31-bit counter
// Formally, the wait ends when the counter drops to zero
//
// The add call should happens before the corresponding done call, otherwise
// the behavior is undefined
//
// The counter increment and wait operations are independent
// (WaitPoint is not one-time),
// so you can wait without even adding new tasks. However, if WaitPoint is
// used to wait for specific tasks to complete, the add call should
// happens before the wait call
class WaitPoint {
private:
	using state_t = ::std::uint64_t;

	enum Mask : state_t {
		counter = 0x7FFF'FFFF,
		helping_bit = 0x8000'0000,
	};

	inline static constexpr auto align =
		::std::hardware_destructive_interference_size;

	// [  32bit  |   bit   |  31bit  ]
	// [ version | helping | counter ]
	// initially, helping bit is set
	alignas(align) ::std::atomic<state_t> state_ = Mask::helping_bit;
	alignas(align) Mutex m_; // protects the condvar
	CondVar counter_is_zero_{m_};

public:
	~WaitPoint() = default;

	WaitPoint(WaitPoint const &) = delete;
	void operator= (WaitPoint const &) = delete;

	WaitPoint(WaitPoint &&) = delete;
	void operator= (WaitPoint &&) = delete;

public:
	WaitPoint() = default;

	// increases the counter by delta
	void add(state_t const delta = 1) noexcept
	{
		// increasing logically
		[[maybe_unused]] auto const prev_state =
			state_.fetch_sub(delta, ::std::memory_order_relaxed);

		UTIL_ASSERT(
			isAddOk(prev_state, delta),
			"increment must be non-zero and "
			"not overflow the 31-bit counter"
		);
	}

	// reduces the counter by delta, (if requested) performs synchronization,
	// and if the counter drops to zero (logically),
	// unblocks all threads currently waiting for
	void done(state_t const delta = 1, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		// reducing logically
		auto const prev_state = state_.fetch_add(delta, order);

		UTIL_ASSERT(
			isDoneOk(prev_state, delta),
			"decrement must be non-zero and "
			"not underflow the 31-bit counter"
		);

		auto const state = prev_state + delta;

		if (isNotifyNeeded(state)) {
			auto lock = ::std::lock_guard(m_);

			counter_is_zero_.notify_all();
		}
	}

	// blocks the current thread until the counter drops to zero, and
	// (if requested) synchronizes with threads that
	// performed synchronization when calling done
	void wait(::std::memory_order order = ::std::memory_order_seq_cst) noexcept
	{
		auto const state = getWaitingState(order);

		if (isCounterZero(state)) [[unlikely]] {
			return;
		}

		auto lock = ::std::lock_guard(m_);

		while (isWaitNeeded(state, order)) {
			counter_is_zero_.wait();
		}
	}

private:
	// State is transient if the counter has dropped to zero and
	// helping_bit isn't set (bc of the carry bit)
	//
	// In this case, progress is needed
	static bool isTransientState(state_t const state) noexcept
	{
		return static_cast<::std::uint32_t>(state) == 0;
	}

	// progress has occurred if the version has not rolled back
	bool tryToProgress(state_t state) noexcept
	{
		auto const version = getVersion(state) - 1;

		UTIL_IGNORE(state_.compare_exchange_strong(
			state,
			setHelpingBit(state),
			::std::memory_order_relaxed
		));

		return getVersion(state) != version;
	}

	// true if the counter has dropped to zero (logically)
	bool isNotifyNeeded(state_t const state) noexcept
	{
		return isTransientState(state) && tryToProgress(state);
	}

	// get the current state, and if necessary help the progress of the system
	// (there is no progress if helping bit is not set)
	state_t getWaitingState(::std::memory_order order) noexcept
	{
		auto state = state_.load(order);

		while (true) {
			if (testHelpingBit(state)) {
				return state;
			}

			auto const new_state = setHelpingBit(state);

			if (state_.compare_exchange_weak(state, new_state, order)) {
				return new_state;
			}

			// backoff is not needed, bc it is not a spinlock, that is,
			// after failed CAS, nothing prevents you from trying to
			// set helping bit immediately again
		}
	}

	// We need to keep waiting if for state_
	// version corresponds, helping bit is set and counter is non-zero, or
	// version is one more, helping bit isn't set, and counter is zero
	//
	// It is checked in one comparison by bit magic/manipulation
	bool isWaitNeeded(state_t const waiting_state,
		::std::memory_order order) const noexcept
	{
		auto const state = state_.load(order);

		return dropCounter(state - 1) == dropCounter(waiting_state);
	}

	////////////////////////////////////////////////////////////////////////////

	// true if the counter does not overflow (logically)
	// after increasing by delta
	static bool isAddOk(state_t const state, state_t const delta) noexcept
	{
		return delta - 1 < onlyCounter(state - 1);
	}

	// true if the counter does not underflow (logically)
	// after reducing by delta
	static bool isDoneOk(state_t const state, state_t const delta) noexcept
	{
		return delta - 1 < onlyCounter(0 - state);
	}

	static state_t onlyCounter(state_t const state) noexcept
	{
		return state & Mask::counter;
	}

	static state_t dropCounter(state_t const state) noexcept
	{
		return state & ~Mask::counter;
	}

	static bool isCounterZero(state_t const state) noexcept
	{
		return onlyCounter(state) == 0;
	}

	static state_t setHelpingBit(state_t const state) noexcept
	{
		return state | Mask::helping_bit;
	}

	static bool testHelpingBit(state_t const state) noexcept
	{
		return (state & Mask::helping_bit) != 0;
	}

	static ::std::uint32_t getVersion(state_t const state) noexcept
	{
		return static_cast<::std::uint32_t>(state >> 32);
	}
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_WAIT_POINT_HPP_INCLUDED_ */
