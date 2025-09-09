// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_COMBINE_PAR_DETAIL_USE_COUNT_H_
#define DDV_EXE_FUTURE_FUN_COMBINE_PAR_DETAIL_USE_COUNT_H_ 1

#include <atomic>
#include <cstdint>
#include <utility>

namespace exe::future::detail {

class UseCount {
public:
	using state_t = ::std::uint64_t;
	using count_t = ::std::uint32_t;

private:
	::std::atomic<state_t> state_;

public:
	explicit UseCount(count_t count) noexcept
		: state_(count)
	{}

	// returns true on the first call
	[[nodiscard]] bool use() noexcept
	{
		auto state = state_.fetch_add(
			state_t(1) << 32,
			::std::memory_order_relaxed
		);

		return notUsed(state);
	}

	// returns information about whether the use call was made,
	// as well as whether this call is the last
	[[nodiscard]] ::std::pair<bool, bool> release(
		::std::memory_order order) noexcept
	{
		auto state = state_.fetch_sub(1, order);

		return {notUsed(state), isLast(state)};
	}

private:
	static bool notUsed(state_t const state) noexcept
	{
		return static_cast<count_t>(state >> 32) == 0;
	}

	static bool isLast(state_t const state) noexcept
	{
		return static_cast<count_t>(state) == 1;
	}
};

} // namespace exe::future::detail

#endif /* DDV_EXE_FUTURE_FUN_COMBINE_PAR_DETAIL_USE_COUNT_H_ */
