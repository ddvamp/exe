//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_ 1

#include <utility>

#include "exe/fiber/sync/condvar.hpp"
#include "exe/fiber/sync/mutex.hpp"

#include "util/debug.hpp"
#include "util/macro.hpp"

namespace exe::fiber {

// Synchronization primitive for waiting for a one-time notification
// and sharing data with it
class Event {
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

		UTIL_ASSERT(
			!::std::exchange(notified_, true),
			"one-time notification sent twice"
		);

		UTIL_IGNORE(its_time_.notify_all());
	}

	// in case of instance reuse
	void clear() noexcept
	{
		notified_ = false;
	}
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_ */
