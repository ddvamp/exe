//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_EVENT_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_EVENT_HPP_INCLUDED_ 1

#include <atomic>

#include "util/debug.hpp"

namespace concurrency {

// Synchronization primitive for waiting for a event and sharing data through it
class Event {
private:
	::std::atomic_flag is_ready_;

public:
	[[nodiscard]] bool isReady() const noexcept
	{
		return is_ready_.test(::std::memory_order_acquire);
	}

	void wait() noexcept
	{
		is_ready_.wait(false, ::std::memory_order_acquire);
	}

	void notify() noexcept
	{
		UTIL_VERIFY(
			!is_ready_.test_and_set(::std::memory_order_release),
			"one-shot event happened twice"
		);

		is_ready_.notify_all();
	}

	// In case of instance reuse
	// Must happens after of any interaction of previous use
	void reset() noexcept
	{
		is_ready_.clear(::std::memory_order_relaxed);
	}
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_EVENT_HPP_INCLUDED_ */
