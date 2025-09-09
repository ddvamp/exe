// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_COMBINE_SEQ_VIA_H_
#define DDV_EXE_FUTURE_FUN_COMBINE_SEQ_VIA_H_ 1

#include <utility>

#include "exe/future/fun/mutator/mutator.h"
#include "exe/future/fun/syntax/pipe.h"

namespace exe::future {

namespace pipe {

class [[nodiscard]] Via : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

private:
	runtime::ISafeScheduler &where_;

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = true;

	explicit Via(runtime::ISafeScheduler &where) noexcept
		: where_(where)
	{}

private:
	template <concepts::Future F>
	auto mutate(F f) noexcept
	{
		return setScheduler(::std::move(f), where_);
	}
};

} // namespace pipe

inline auto via(runtime::ISafeScheduler &where) noexcept
{
	return pipe::Via(where);
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURE_FUN_COMBINE_SEQ_VIA_H_ */
