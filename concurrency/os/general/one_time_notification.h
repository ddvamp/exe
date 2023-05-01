// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_OS_GENERAL_ONE_TIME_NOTIFICATION_H_
#define DDV_CONCURRENCY_OS_GENERAL_ONE_TIME_NOTIFICATION_H_ 1

#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <utility>

#include "utils/debug.h"

namespace concurrency {

// Synchronization primitive for waiting for a one-time notification
// and sharing data with it
class OneTimeNotification {
private:
	using state_t = ::std::uint64_t;

	enum State : state_t {
		initial,
		notified,
	};

	state_t state_ = State::initial;
	::std::mutex m_; // protects the condvar
	::std::condition_variable its_time_;

public:
	void wait()
	{
		auto lock = ::std::unique_lock(m_);

		its_time_.wait(
			lock,
			[this]() noexcept {
				return state_ == State::notified;
			}
		);
	}

	void notify()
	{
		{
			auto lock = ::std::lock_guard(m_);

			UTILS_ASSERT(
				::std::exchange(state_, State::notified) == State::initial,
				"one-time notification sent twice"
			);
		}

		its_time_.notify_all();
	}

	// in case of instance reuse
	void clear() noexcept
	{
		state_ = State::initial;
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_OS_GENERAL_ONE_TIME_NOTIFICATION_H_ */
