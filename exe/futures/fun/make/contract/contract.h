// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_CONTRACT_CONTRACT_H_
#define DDV_EXE_FUTURES_FUN_MAKE_CONTRACT_CONTRACT_H_ 1

#include <utility>

#include "exe/futures/fun/make/contract/fwd.h"
#include "exe/futures/fun/state/shared_state.h"
#include "exe/futures/fun/types/future.h"

#include "result/result.h"

namespace exe::futures {

template <typename T>
class Promise : protected detail::HoldState<T> {
	friend Contract<T>;

private:
	using Base = detail::HoldState<T>;

	using Base::Base;

	using Base::release;

public:
	using typename Base::Result;

public:
	void setResult(Result result) && noexcept
	{
		release()->setResult(::std::move(result));
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

	~Contract()
	{
		if (f.hasState()) [[unlikely]] {
			cancel();
		}
	}

	Contract()
		: Contract(State::create())
	{}

private:
	Contract(State *state) noexcept
		: f(state)
		, p(state)
	{}

	void cancel() noexcept
	{
		State::destroy(f.release());
		p.reset();
	}
};

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_CONTRACT_CONTRACT_H_ */
