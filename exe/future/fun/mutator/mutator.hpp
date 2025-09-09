// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_MUTATOR_MUTATOR_H_
#define DDV_EXE_FUTURE_FUN_MUTATOR_MUTATOR_H_ 1

#include <utility>

#include "exe/future/fun/mutator/fwd.hpp"
#include "exe/future/fun/types/future.hpp"

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
		if constexpr (has_scheduler_v<F>) {
			return FutureHolder(f);
		} else {
			return SemiFutureHolder(f);
		}
	}

	template <typename F>
	static void makeHolder(SemiFutureHolder<F> &) noexcept = delete;



	template <concepts::Future F>
	[[nodiscard]] static runtime::ISafeScheduler &getScheduler(F const &f)
		noexcept
		requires (has_scheduler_v<F>)
	{
		return f.getState().getScheduler();
	}

	template <typename F>
	[[nodiscard]] static runtime::ISafeScheduler &getScheduler(
		FutureHolder<F> const &f) noexcept
	{
		return f.raw.getState().getScheduler();
	}



	template <concepts::Future F>
	static auto setScheduler(F f, runtime::ISafeScheduler &where) noexcept
	{
		f.getState().setScheduler(where);
		return F::with_scheduler(f.release());
	}

	template <typename F>
	static auto setScheduler(SemiFutureHolder<F> f,
		runtime::ISafeScheduler &where) noexcept
	{
		f.raw.getState().setScheduler(where);
		return FutureHolder<F>(f.raw);
	}



	template <concepts::Future F>
	static auto unsetScheduler(F f) noexcept
	{
		return F::without_scheduler(::std::move(f));
	}

	template <typename F>
	static auto unsetScheduler(SemiFutureHolder<F> f) noexcept
	{
		return ::std::move(f);
	}



	template <concepts::Future F>
	static void setCallback(F f, F::Callback &&cb) noexcept
		requires (has_scheduler_v<F>)
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

#endif /* DDV_EXE_FUTURE_FUN_MUTATOR_MUTATOR_H_ */
