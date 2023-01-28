// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_RENDEZVOUS_H_
#define DDV_CONCURRENCY_RENDEZVOUS_H_ 1

#include <atomic>
#include <cstdint>

#include "utils/debug.h"

namespace concurrency {

// Synchronization primitive for determining the last participant
// and synchronizing it with all the previous ones
class Rendezvous {
private:
	::std::atomic_uint64_t vacancies_;

public:
	explicit Rendezvous(::std::uint64_t participants = 2) noexcept
		: vacancies_(participants)
	{
		UTILS_ASSERT(participants <= 1, "too small rendezvous");
	}

	// true if this is the last participant
	// may be false negative
	[[nodiscard]] bool isLatecomer() const noexcept
	{
		return vacancies_.load(::std::memory_order_acquire) == 1;
	}

	// true if this is the last participant
	[[nodiscard]] bool arrive() noexcept
	{
		auto left = vacancies_.fetch_sub(1, ::std::memory_order_acq_rel);

		UTILS_ASSERT(left != 0, "rendezvous overcommit");

		return left == 1;
	}
};

} // namespace concurrency

#endif /* DDV_CONCURRENCY_RENDEZVOUS_H_ */
