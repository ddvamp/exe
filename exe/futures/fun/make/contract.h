// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_CONTRACT_H_
#define DDV_EXE_FUTURES_FUN_MAKE_CONTRACT_H_ 1

#include <type_traits>
#include <utility>

#include "exe/futures/fun/state/shared_state.h"
#include "exe/futures/fun/types/future.h"

#include "result/result.h"

#include "utils/macro.h"

namespace exe::futures {

template <typename T>
class Promise : protected detail::HoldState<T> {
	template <typename>
	friend struct Contract;

private:
	using Base = detail::HoldState<T>;

	using Base::Base;

	using Base::getState;
	using Base::release;
	using Base::reset;

	using Base::Result;

public:
	void setResult(Result result) &&
		noexcept (::std::is_nothrow_move_constructible_v<Result>)
	{
		// strong exception safety
		getState().setResult(::std::move(result));
		reset();
	}

	void setError(::utils::error error) && noexcept
	{
		release()->setResult(::std::move(error));
	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct [[nodiscard]] Contract {
	using State = detail::SharedState<T>;

	SemiFuture<T> f;
	Promise<T> p;

	Contract()
		: Contract(State::create())
	{}

	void cancel() && noexcept
	{
		UTILS_IGNORE(f.release());
		State::destroy(p.release());
	}

private:
	Contract(State *state) noexcept
		: f(state)
		, p(state)
	{}
};

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_CONTRACT_H_ */
