// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_WAIT_POINT_H_
#define DDV_CONCURRENCY_WAIT_POINT_H_ 1

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <new>

#include <utils/debug.h>
#include <utils/macro.h>
#include <utils/utility.h>

// TODO: Better documentations
// TODO: Tests

namespace concurrency {

// TODO: Better naming
// TODO: Move out
struct Enforce {
	explicit Enforce() = default;
};

inline constexpr Enforce enforce{};

////////////////////////////////////////////////////////////////////////////////

// Synchronization primitive for waiting for the completion of tasks,
// which are expressed as a 31-bit counter. Formally, the wait ends when
// the counter drops to zero (with various reservations)
// 
// The counter increments and wait operations are independent
// (WaitPoint is not one-time), so you can wait without even adding new tasks.
// However, if WaitPoint is used to wait for specific tasks to complete,
// the Add() call should happens before the Wait() call
class WaitPoint {
private:
	using State = ::std::uint64_t;

	enum Mask : State {
		counter = 0x7FFF'FFFF,
		helping_bit = 0x8000'0000,
	};

	// TODO: Is it overly complex?
	class Token {
		friend WaitPoint;

	private:
		State state_;

	public:
		~Token() = default;

		Token(Token const &) = delete;
		void operator= (Token const &) = delete;

		Token(Token &&that) noexcept
			: state_(that.state_) {
			that.Use();
		}
		void operator= (Token &&) = delete;

	private:
		explicit Token(State const state) noexcept
			: state_(state) {}

		[[nodiscard]] State GetState() const noexcept {
			return state_;
		}

		void Use() noexcept {
			state_ = Mask::helping_bit;
		}
	};

	inline static constexpr auto align =
		::std::hardware_destructive_interference_size;

	// [ 32 bits |  1 bit  | 31 bits ]
	// [ version | helping | counter ] (Initially helping bit is set)
	alignas(align) ::std::atomic<State> state_;
	alignas(align) ::std::mutex m_; // Protects the condvar
	::std::condition_variable counter_is_zero_;

public:
	~WaitPoint() = default;

	WaitPoint(WaitPoint const &) = delete;
	void operator= (WaitPoint const &) = delete;

	WaitPoint(WaitPoint &&) = delete;
	void operator= (WaitPoint &&) = delete;

public:
	// Throws: system_error
	// Error conditions:
	//  -  resource_unavailable_try_again (condvar)
	explicit WaitPoint(State const already_have = 0)
		: state_(SetHelpingBit(already_have)) {
		UTILS_ASSERT(
			already_have < Mask::helping_bit,
			"Initializer does not fit into the counter"
		);
	}

	// Increases the counter by delta
	Token Add(State const delta = 1) noexcept {
		auto const state = state_.fetch_add(delta, ::std::memory_order_relaxed);
		UTILS_ASSERT(
			IsAddCorrect(state, delta),
			"Increment must be non-zero and "
			"not overflow the 31-bit counter"
		);

		return Token(state + delta);
	}

	// Reduces the counter by delta, performs synchronization if requested, and
	// if the counter drops to zero, unblocks all threads currently waiting for
	//
	// Corresponding Add() call must happen before this call
	Token Done(State const delta = 1, ::std::memory_order const order =
		::std::memory_order_seq_cst) {
		auto const state = state_.fetch_sub(delta, order);
		UTILS_ASSERT(
			IsDoneCorrect(state, delta),
			"Decrement must be non-zero and "
			"not underflow the 31-bit counter"
		);

		if (EnsureNotify(state - delta)) {
			// It is necessary to make sure that no Wait() call is
			// trying to wait on condvar
			// 
			// Bad scenario:
			//   EnsureWait() -> Pred() -> EnsureNotify() ->
			//   notify_all() -> wait()
			//
			// Throws: system_error (not permitted/deadlock)
			// TODO: Exception handling
			UTILS_IGNORE(::std::lock_guard(m_)); // Intentionally
			counter_is_zero_.notify_all();
		}

		return Token(state - delta);
	}

	// Blocks the current thread until the counter drops to zero, and
	// if requested, synchronizes with previous Done() calls. Returns true if
	// the wait has occurred, but sometimes it can be false positive
	//
	// This call should happen after all Add() calls corresponding to
	// Done() calls for which it wants to wait
	//
	// Throws: system_error
	// Error conditions:
	//  -  operation_not_permitted (mutex)
	//  �  resource_deadlock_would_occur (mutex) // Not possible!
	bool Wait(::std::memory_order const order = ::std::memory_order_seq_cst) {
		return WaitImpl(state_.load(order), order);
	}

	// This call can be used independently of any Add() calls
	//
	// Throws: system_error
	// Error conditions:
	//  -  operation_not_permitted (mutex)
	//  �  resource_deadlock_would_occur (mutex) // Not possible!
	// 
	// TODO: Is it needed?
	bool Wait(Enforce, ::std::memory_order const order =
		::std::memory_order_seq_cst) {
		return WaitImpl(state_.fetch_add(0, order), order);
	}

	// The call ends immediately if the counter has already dropped to zero
	// since receiving the token
	// 
	// Reusing the token does not result in waiting
	//
	// Throws: system_error
	// Error conditions:
	//  -  operation_not_permitted (mutex)
	//  �  resource_deadlock_would_occur (mutex) // Not possible!
	bool Wait(Token &&token, ::std::memory_order const order =
		::std::memory_order_seq_cst) {
		auto const res = WaitImpl(token.GetState(), state_.load(order), order);
		token.Use();
		return res;
	}

private:
	bool WaitImpl(State const state, ::std::memory_order const order) {
		return WaitImpl(state, state, order);
	}

	bool WaitImpl(State const first_state, State const state,
		::std::memory_order const order) {
		if (EnsureWait(first_state, state, order)) {
			counter_is_zero_.wait(
				::utils::temporary(::std::unique_lock(m_)),
				[=, version = state & 0xFFFF'FFFF'0000'0000ull]() noexcept {
					// We need to keep waiting if for state_
					// helping bit is zero and version corresponds
					return (state_.load(order) & ~Mask::counter) != version;
				}
			);
			return true;
		}
		return false;
	}

	// Checks if it is needed to wait. In some cases, it is necessary
	// to clarify the state by setting helping bit
	[[nodiscard]] bool EnsureWait(State const first_state, State state,
		::std::memory_order const order) noexcept {
		do {
			auto const [is_help_needed, is_wait_needed] =
				CheckHelpWait(first_state, state);
			if (!is_help_needed) {
				return is_wait_needed;
			}

			// Backoff is not needed because it is not a spinlock, that is,
			// after a CAS failure, nothing prevents you from
			// immediately trying to set helping bit again
		} while (
			!state_.compare_exchange_weak(state, SetHelpingBit(state), order)
		);

		return GetStage(state) != 0x0000'0000u;
	}

	// Checks if the counter has dropped to zero, and if so,
	// tries to update the stage. If successful, notify should be sent
	[[nodiscard]] bool EnsureNotify(State state) noexcept {
		auto const [version, stage] = GetVersionStage(state);
		if (stage != 0x0000'0000u) {
			return false;
		}

		auto const success = state_.compare_exchange_strong(
			state,
			SetHelpingBit(state),
			::std::memory_order_relaxed
		);

		return ::utils::any_of(success, GetVersion(state) != version);
	}

	// Checks if the counter is not overflowing after increasing by delta, and
	// that the delta fits into the counter and is not zero
	[[maybe_unused, nodiscard]] static bool IsAddCorrect(State const state,
		State const delta) noexcept {
		// cnt + delta <= MAX && delta != 0
		return delta - 1 < (~state & Mask::counter);
	}

	// Checks if the counter is not underflowing after decreasing by delta, and
	// that the delta fits into the counter and is not zero
	[[maybe_unused, nodiscard]] static bool IsDoneCorrect(State const state,
		State const delta) noexcept {
		// cnt - delta >= 0 && delta != 0
		return delta - 1 < (state & Mask::counter);
	}

	// Determines whether to help with the stage update, start wait or leave.
	// It is performed through bit magic
	// 
	// TODO: Add detail description of stages
	[[nodiscard]] static ::std::pair<bool, bool> CheckHelpWait(
		State const fst, State const snd) noexcept {
		auto const [fst_v, fst_s] = GetVersionStage(fst);
		auto const [snd_v, snd_s] = GetVersionStage(snd);

		int A = fst_v == snd_v;

		int B = fst_s > 0x8000'0000u;
		int C = fst_s != 0x8000'0000u;

		int D = snd_s > 0x8000'0000u;
		int E = snd_s < 0x8000'0000u;

		int F = snd_s == 0x0000'0000u;
		int G = snd_s != 0x0000'0000u;

		return {(A & C) & ((B & D) | F), (A & C) & (E & G)};
	}

	[[nodiscard]] static State SetHelpingBit(State const state) noexcept {
		return state - Mask::helping_bit;
	}

	[[nodiscard]] static ::std::uint32_t GetVersion(
		State const state) noexcept {
		return static_cast<::std::uint32_t>(state >> 32);
	}

	[[nodiscard]] static ::std::uint32_t GetStage(State const state) noexcept {
		return static_cast<::std::uint32_t>(state);
	}

	[[nodiscard]] static ::std::pair<::std::uint32_t, ::std::uint32_t>
		GetVersionStage(State const state) noexcept {
		return {GetVersion(state), GetStage(state)};
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_WAIT_POINT_H_ */
