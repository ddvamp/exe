// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MUTATOR_MUTATOR_H_
#define DDV_EXE_FUTURES_FUN_MUTATOR_MUTATOR_H_ 1

#include <utility>

#include "exe/future/fun/mutator/fwd.h"
#include "exe/future/fun/types/future.h"

namespace exe::future::detail {

class Mutator {
protected:
	Mutator() = default;
	~Mutator() = default;

	Mutator(Mutator const &) = delete;
	void operator= (Mutator const &) = delete;

	Mutator(Mutator &&) = delete;
	void operator= (Mutator &&) = delete;

protected:
	template <concepts::Future F>
	static auto makeHolder(F &f) noexcept
	{
		if constexpr (has_executor_v<F>) {
			return FutureHolder(f);
		} else {
			return SemiFutureHolder(f);
		}
	}

	template <typename F>
	static void makeHolder(SemiFutureHolder<F> &) noexcept = delete;



	template <concepts::Future F>
	[[nodiscard]] static executors::INothrowExecutor &getExecutor(F const &f)
		noexcept
		requires (has_executor_v<F>)
	{
		return f.getState().getExecutor();
	}

	template <typename F>
	[[nodiscard]] static executors::INothrowExecutor &getExecutor(
		FutureHolder<F> const &f) noexcept
	{
		return f.raw.getState().getExecutor();
	}



	template <concepts::Future F>
	static auto setExecutor(F f, executors::INothrowExecutor &where) noexcept
	{
		f.getState().setExecutor(where);
		return F::with_executor(f.release());
	}

	template <typename F>
	static auto setExecutor(SemiFutureHolder<F> f,
		executors::INothrowExecutor &where) noexcept
	{
		f.raw.getState().setExecutor(where);
		return FutureHolder<F>(f.raw);
	}



	template <concepts::Future F>
	static auto unsetExecutor(F f) noexcept
	{
		return F::without_executor(::std::move(f));
	}

	template <typename F>
	static auto unsetExecutor(SemiFutureHolder<F> f) noexcept
	{
		return ::std::move(f);
	}



	template <concepts::Future F>
	static void setCallback(F f, F::Callback &&cb) noexcept
		requires (has_executor_v<F>)
	{
		f.release()->setCallback(::std::move(cb));
	}

	template <typename F>
	static void setCallback(FutureHolder<F> f, F::Callback &&cb) noexcept
	{
		f.raw.release()->setCallback(::std::move(cb));
	}
};

} // namespace exe::future::detail

#endif /* DDV_EXE_FUTURES_FUN_MUTATOR_MUTATOR_H_ */
