// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_FUTURE_H_
#define DDV_EXE_FUTURES_FUN_FUTURE_H_ 1

#include <utility>

#include "exe/futures/fun/state/shared_state.h"
#include "exe/futures/fun/promise.h"

#include "result/result.h"

namespace exe::futures {

template <typename T>
class [[nodiscard]] SemiFuture : protected detail::HoldState<T> {
	friend class Contract;
	friend class Mutator;

protected:
	using Base = detail::HoldState<T>;

	using Base::Base;

public:
	using Base::value_type;
};

// Class for representing future value
// Future is moveable value type
template <typename T>
class [[nodiscard]] Future : public SemiFuture<T> {
	friend Mutator;

protected:
	using SemiFuture<T>::SemiFuture;
};


////////////////////////////////////////////////////////////////////////////////


struct Contract {
	template <typename T>
	struct [[nodiscard]] Content {
		SemiFuture<T> future;
		Promise<T> promise;
	};

	template <::utils::suitable_for_result T>
	static Content<T> make()
	{
		auto state = detail::SharedState<T>::create();
		return {SemiFuture<T>(state), Promise<T>(state)};
	}
};

struct Mutator {
protected:
	template <typename T>
	[[nodiscard]] static executors::IExecutor &getExecutor(
		Future<T> const &f) noexcept
	{
		return f.getState().getExecutor();
	}

	template <typename T>
	static Future<T> setExecutor(SemiFuture<T> f,
		executors::IExecutor &where) noexcept
	{
		f.getState().setExecutor(where);
		return Future<T>(f.release());
	}

	template <typename T>
	static void setCallback(Future<T> f, Future<T>::Callback &&cb) noexcept
	{
		f.release()->setCallback(::std::move(cb));
	}
};

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_FUTURE_H_ */
