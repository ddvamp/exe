// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_SPINLOCK_H_
#define DDV_CONCURRENCY_SPINLOCK_H_ 1

#include <atomic>

namespace concurrency {

class Spinlock {
private:
	::std::atomic_flag locked_;

public:
	[[nodiscard]] bool try_lock() noexcept
	{
		return !locked_.test_and_set(::std::memory_order_acquire);
	}

	void lock() noexcept
	{
		while (locked_.test_and_set(::std::memory_order_acquire)) {
			while (locked_.test(::std::memory_order_relaxed)) {
				// backoff
			}
		}
	}

	void unlock() noexcept
	{
		locked_.clear(::std::memory_order_release);
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_SPINLOCK_H_ */
