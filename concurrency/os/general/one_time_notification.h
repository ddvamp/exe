// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_OS_GENERAL_ONE_TIME_NOTIFICATION_H_
#define DDV_CONCURRENCY_OS_GENERAL_ONE_TIME_NOTIFICATION_H_ 1

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <utility>

#include "utils/debug.h"

namespace concurrency {

// Synchronization primitive for waiting for a one-time notification
// and sharing data with it
class OneTimeNotification {
private:
	::std::atomic_bool ready_ = false;
	::std::mutex m_; // protects the condvar
	::std::condition_variable its_time_;

public:
	[[nodiscard]] bool isReady() const noexcept
	{
		return ready_.load(::std::memory_order_acquire);
	}

	void wait()
	{
		if (isReady()) {
			return;
		}

		auto lock = ::std::unique_lock(m_);

		while (notReady()) {
			its_time_.wait(lock);
		}
	}

	void notify()
	{
		{
			auto lock = ::std::lock_guard(m_);

			UTILS_ASSERT(notReady(), "one-time notification sent twice");

			ready_.store(true, ::std::memory_order_release);
		}

		its_time_.notify_all();
	}

	// in case of instance reuse
	void reset() noexcept
	{
		ready_.store(false, ::std::memory_order_relaxed);
	}

private:
	bool notReady() const noexcept
	{
		return !ready_.load(::std::memory_order_relaxed);
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_OS_GENERAL_ONE_TIME_NOTIFICATION_H_ */
