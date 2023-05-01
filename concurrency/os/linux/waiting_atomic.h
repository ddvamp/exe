// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_OS_LINUX_WAITING_ATOMIC_H_
#define DDV_CONCURRENCY_OS_LINUX_WAITING_ATOMIC_H_ 1

#include <atomic>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <memory>

#include <linux/futex.h>	// FUTEX_* constants
#include <sys/syscall.h>	// SYS_* constants
#include <unistd.h>

#include "utils/debug.h"

namespace concurrency {

using wait_t = ::std::uint32_t;

namespace detail {

inline void futex_wait(wait_t const *uaddr, wait_t val,
	::std::timespec const *timeout) noexcept
{
	[[maybe_unused]] auto error = syscall(
		SYS_futex,
		uaddr,
		FUTEX_WAIT_PRIVATE,
		val,
		timeout,
		nullptr,
		0
	);

	UTILS_CHECK(
		error == 0 ||		// the caller was woken up (maybe spuriously)
		errno == EAGAIN ||	// the futex value does not match val on check
		errno == EINTR,		// interrupted by a signal (spurious wakeup)
		"error when calling futex_wait"
	);
}

inline ::std::size_t futex_wake(wait_t const *uaddr, wait_t val) noexcept
{
	auto awakened = syscall(
		SYS_futex,
		uaddr,
		FUTEX_WAKE_PRIVATE,
		val,
		nullptr,
		nullptr,
		0
	);

	UTILS_ASSUME(awakened >= 0, "error when calling futex_wake");

	return static_cast<::std::size_t>(awakened);
}

} // namespace concurrency::detail


////////////////////////////////////////////////////////////////////////////////


class NotifyToken {
private:
	friend class WaitingAtomic;

	using value_type = wait_t;

	value_type const *address;

private:
	explicit NotifyToken(value_type const *p) noexcept
		: address(p)
	{}

public:
	// up to INT_MAX waiters
	::std::size_t notify_n(value_type count) const noexcept
	{
		return detail::futex_wake(address, normalize_count(count));
	}

	::std::size_t notify_one() const noexcept
	{
		return notify_n(1);
	}

	::std::size_t notify_all() const noexcept
	{
		return notify_n(INT_MAX);
	}

private:
	inline static value_type normalize_count(value_type count) noexcept
	{
		return count & 0x8000'0000u;
	}
};

// atomic_uint32_t with an efficient OS-specific wait/notification mechanism
class WaitingAtomic {
private:
	using clock_t = ::std::chrono::steady_clock;

	using value_type = wait_t;

	::std::atomic<value_type> atomic_ = 0;

public:
	inline static constexpr bool is_always_lock_free =
		::std::atomic<value_type>::is_always_lock_free;

	[[nodiscard]] bool is_lock_free() const noexcept
	{
		return atomic_.is_lock_free();
	}

public:
	constexpr WaitingAtomic() noexcept = default;
	~WaitingAtomic() noexcept = default;

	WaitingAtomic(WaitingAtomic const &) = delete;
	void operator= (WaitingAtomic const &) = delete;

	WaitingAtomic(WaitingAtomic &&) = delete;
	void operator= (WaitingAtomic &&) = delete;

public:
	constexpr WaitingAtomic(value_type val) noexcept
		: atomic_(val)
	{}

	value_type operator= (value_type val) noexcept
	{
		atomic_.store(val);
		return val;
	}

	void store(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		atomic_.store(val, order);
	}

	value_type load(::std::memory_order order =
		::std::memory_order_seq_cst) const noexcept
	{
		return atomic_.load(order);
	}

	operator value_type() const noexcept
	{
		return atomic_.load();
	}

	value_type exchange(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst)
	{
		return atomic_.exchange(val, order);
	}

	bool compare_exchange_weak(value_type &expected, value_type desired,
		::std::memory_order success, ::std::memory_order failure) noexcept
	{
		return atomic_.compare_exchange_weak(
			expected, desired, success, failure
		);
	}

	bool compare_exchange_weak(value_type &expected, value_type desired,
		::std::memory_order order = ::std::memory_order_seq_cst) noexcept
	{
		return atomic_.compare_exchange_weak(expected, desired, order);
	}

	bool compare_exchange_strong(value_type &expected, value_type desired,
		::std::memory_order success, ::std::memory_order failure) noexcept
	{
		return atomic_.compare_exchange_strong(
			expected, desired, success, failure
		);
	}

	bool compare_exchange_strong(value_type &expected, value_type desired,
		::std::memory_order order = ::std::memory_order_seq_cst) noexcept
	{
		return atomic_.compare_exchange_strong(expected, desired, order);
	}

	void wait(value_type old, ::std::memory_order order =
		::std::memory_order_seq_cst) const noexcept
	{
		while (atomic_.load(order) == old) {
			detail::futex_wait(raw_address(), old, nullptr);
		}
	}

	// true on wakeup
	template <typename Duration>
	bool wait_until(value_type old, ::std::memory_order order,
		::std::chrono::time_point<clock_t, Duration> const &time) noexcept
	{
		using namespace ::std::chrono;

		while (atomic_.load(order) == old) {
			auto const now = clock_t::now();

			if (now >= time) {
				return false;
			}

			auto const delta = time - now;
			auto const s = duration_cast<seconds>(delta);
			auto const ns = duration_cast<nanoseconds>(delta) -
				duration_cast<nanoseconds>(s);

			auto const timeout = ::std::timespec{
				.tv_sec = s.count(),
				.tv_nsec = ns.count(),
			};

			detail::futex_wait(raw_address(), old, &timeout);
		}

		return true;
	}

	// true on wakeup
	template <typename Clock, typename Duration>
	bool wait_until(value_type old, ::std::memory_order order,
		::std::chrono::time_point<Clock, Duration> const &time) noexcept
	{
		auto const now = Clock::now();
		auto const delta = time - now;

		return wait_for(old, order, delta);
	}

	// true on wakeup
	template <typename Rep, typename Period>
	bool wait_for(value_type old, ::std::memory_order order,
		::std::chrono::duration<Rep, Period> &delta) noexcept
	{
		auto const s_now = clock_t::now();
		auto const s_time = s_now +
			::std::chrono::ceil<clock_t::duration>(delta);

		return wait_until(old, order, s_time);
	}

	// up to INT_MAX waiters
	::std::size_t notify_n(value_type count) const noexcept
	{
		return get_notify_token().notify_n(count);
	}

	::std::size_t notify_one() const noexcept
	{
		return notify_n(1);
	}

	::std::size_t notify_all() const noexcept
	{
		return notify_n(INT_MAX);
	}

	value_type fetch_add(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		return atomic_.fetch_add(val, order);
	}

	value_type fetch_sub(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		return atomic_.fetch_sub(val, order);
	}

	value_type fetch_and(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		return atomic_.fetch_and(val, order);
	}

	value_type fetch_or(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		return atomic_.fetch_or(val, order);
	}

	value_type fetch_xor(value_type val, ::std::memory_order order =
		::std::memory_order_seq_cst) noexcept
	{
		return atomic_.fetch_xor(val, order);
	}

	value_type operator++ () noexcept
	{
		return ++atomic_;
	}

	value_type operator++ (int) noexcept
	{
		return atomic_++;
	}

	value_type operator-- () noexcept
	{
		return --atomic_;
	}

	value_type operator-- (int) noexcept
	{
		return atomic_--;
	}

	value_type operator+= (value_type val) noexcept
	{
		return atomic_ += val;
	}

	value_type operator-= (value_type val) noexcept
	{
		return atomic_ -= val;
	}

	value_type operator&= (value_type val) noexcept
	{
		return atomic_ &= val;
	}

	value_type operator|= (value_type val) noexcept
	{
		return atomic_ |= val;
	}

	value_type operator^= (value_type val) noexcept
	{
		return atomic_ ^= val;
	}

	NotifyToken get_notify_token() const noexcept
	{
		return NotifyToken(raw_address());
	}

private:
	value_type const *raw_address() const noexcept
	{
		return reinterpret_cast<value_type const *>(::std::addressof(atomic_));
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_OS_LINUX_WAITING_ATOMIC_H_ */
