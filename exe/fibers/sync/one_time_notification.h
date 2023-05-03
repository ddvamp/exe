// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBERS_SYNC_ONE_TIME_NOTIFICATION_H_
#define DDV_EXE_FIBERS_SYNC_ONE_TIME_NOTIFICATION_H_ 1

#include <utility>

#include "exe/fibers/sync/condvar.h"
#include "exe/fibers/sync/mutex.h"

#include "utils/debug.h"
#include "utils/macro.h"

namespace exe::fibers {

// Synchronization primitive for waiting for a one-time notification
// and sharing data with it
class OneTimeNotification {
private:
	bool notified_ = false;
	Mutex m_; // protects the condvar
	CondVar its_time_{m_};

public:
	void wait() noexcept
	{
		auto lock = ::std::lock_guard(m_);

		while (!notified_) {
			its_time_.wait();
		}
	}

	void notify() noexcept
	{
		auto lock = ::std::lock_guard(m_);

		UTILS_ASSERT(
			!::std::exchange(notified_, true),
			"one-time notification sent twice"
		);

		UTILS_IGNORE(its_time_.notify_all());
	}

	// in case of instance reuse
	void clear() noexcept
	{
		notified_ = false;
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_ONE_TIME_NOTIFICATION_H_ */
