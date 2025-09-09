//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_ 1

#include <atomic>

#include "pause.hpp"

namespace concurrency {

class Spinlock {
private:
	::std::atomic_flag locked_;

public:
	[[nodiscard]] bool try_lock() noexcept
	{
		return !locked_.test(::std::memory_order_relaxed) &&
			!locked_.test_and_set(::std::memory_order_acquire);
	}

	void lock() noexcept
	{
		while (!try_lock()) {
			thread_relax();
		}
	}

	void unlock() noexcept
	{
		locked_.clear(::std::memory_order_release);
	}
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_ */
