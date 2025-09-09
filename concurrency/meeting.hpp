// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_MEETING_H_
#define DDV_CONCURRENCY_MEETING_H_ 1

#include <atomic>
#include <cstdint>

#include "util/debug.hpp"

namespace concurrency {

// Synchronization primitive for determining the last participant
// and synchronizing it with all the previous ones
class Meeting {
private:
	using seat_t = ::std::uint64_t;

	::std::atomic<seat_t> seats_;

public:
	// sets the number of calls (participants)
	explicit Meeting(seat_t participants) noexcept
		: seats_(participants)
	{
		UTIL_ASSERT(participants > 1, "too few participants");
	}

	// syncs with previous calls and
	// returns true if this is the last one (participant)
	[[nodiscard]] bool takesPlace() noexcept
	{
		auto left = seats_.fetch_sub(1, ::std::memory_order_acq_rel);

		UTIL_ASSERT(left > 0, "meeting has already taken place");

		return left == 1;
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_MEETING_H_ */
