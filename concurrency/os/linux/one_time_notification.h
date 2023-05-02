// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_OS_LINUX_ONE_TIME_NOTIFICATION_H_
#define DDV_CONCURRENCY_OS_LINUX_ONE_TIME_NOTIFICATION_H_ 1

#include <atomic>

#include "concurrency/os/linux/waiting_atomic.h"

#include "utils/debug.h"
#include "utils/macro.h"

namespace concurrency {

// Synchronization primitive for waiting for a one-time notification
// and sharing data with it
class OneTimeNotification {
private:
	enum State : wait_t {
		initial,
		notified,
	};

	WaitingAtomic state_ = State::initial;

public:
	void wait() noexcept
	{
		state_.wait(State::initial, ::std::memory_order_acquire);
	}

	void notify() noexcept
	{
		UTILS_ASSERT(
			state_.load(::std::memory_order_relaxed) == State::initial,
			"one-time notification sent twice"
		);

		auto token = state_.get_notify_token();

		state_.store(State::notified, ::std::memory_order_release);

		UTILS_IGNORE(token.notify_all());
	}

	// in case of instance reuse
	void clear() noexcept
	{
		state_.store(State::initial, ::std::memory_order_relaxed);
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_OS_LINUX_ONE_TIME_NOTIFICATION_H_ */
