// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_CORE_PROMISE_H_
#define DDV_EXE_FUTURES_CORE_PROMISE_H_ 1

#include <type_traits>

#include "exe/futures/core/detail/shared_state.h"

#include "result/result.h"

#include "utils/utility.h"

namespace exe::futures {

template <typename T>
class Promise : protected detail::HoldState<T> {
	friend class Contract;

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

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_CORE_PROMISE_H_ */
