// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_CORE_FUTURE_H_
#define DDV_EXE_FUTURES_CORE_FUTURE_H_ 1

#include <utility>

#include "exe/executors/execute.h"
#include "exe/executors/inline.h"
#include "exe/futures/core/detail/shared_state.h"
#include "exe/futures/core/promise.h"

#include "result/result.h"

#include "utils/type_traits.h"

namespace exe::futures {

// class for representing future value
template <typename>
class Future;

} // namespace exe::futures


////////////////////////////////////////////////////////////////////////////////


namespace utils {

template <typename T>
inline constexpr bool is_future_v =
	is_specialization_v<::std::remove_cv_t<T>, ::exe::futures::Future>;

template <typename T>
struct is_future : ::std::bool_constant<is_future_v<T>> {};

} // namespace utils


////////////////////////////////////////////////////////////////////////////////


namespace exe::futures {

template <typename F, typename T>
concept SyncContinuation =
	::std::is_invocable_v<F &, T> &&
	!::utils::is_future_v<::utils::detail::result_<F &, T>>;

template <typename F, typename T>
concept AsyncContinuation =
	::std::is_invocable_v<F &, T> &&
	::utils::is_future_v<::utils::detail::result_<F &, T>>;

template <typename F, typename T>
concept ErrorHandler =
	::std::is_invocable_v<F &, ::utils::error> &&
	::std::is_same_v<
		::utils::detail::result_<F &, ::utils::error>,
		::utils::result<T>
	>;


////////////////////////////////////////////////////////////////////////////////


template <typename T>
struct [[nodiscard]] Contract {
	Future<T> future_;
	Promise<T> promise_;
};


////////////////////////////////////////////////////////////////////////////////


template <typename T>
class [[nodiscard]] Future : protected detail::HoldState<T> {
	template <::utils::suitable_for_result U>
	friend Contract<U> makeContractVia(executors::IExecutor &);

private:
	using Base = detail::HoldState<T>;

	using Base::Base;

	using Base::hasState;
	using Base::getState;
	using Base::release;
	using Base::reset;

	using Base::Result;
	using Base::Callback;

public:
	using Base::value_type;

	[[nodiscard]] executors::IExecutor &getExecutor() const noexcept
	{
		return getState().getExecutor();
	}

	Future via(executors::IExecutor &where) && noexcept
	{
		getState().setExecutor(where);
		return Future(release());
	}

	void subscribe(Callback &&callback) && noexcept
	{
		release()->setCallback(::std::move(callback));
	}

	void discard() && noexcept
	{
		release()->setCallback([](Result &&) noexcept {});
	}

	template <SyncContinuation<T> F>
	auto then(F &&cont) &&
	{
		using U = ::utils::detail::result_<F &, T>;

		auto [future, promise] = futures::makeContractVia<U>(getExecutor());

		::std::move(*this).subscribe(
			[f = ::std::forward<F>(cont), p = ::std::move(promise)]
			(Result &&res) mutable noexcept {
				if constexpr (
					::std::is_nothrow_invocable_v<F &, T> &&
					::std::is_nothrow_move_constructible_v<::utils::result<U>>
				) {
					::std::move(p).setResult(::std::move(res).transform(f));
				} else try {
					::std::move(p).setResult(::std::move(res).transform(f));
				} catch (...) {
					::std::move(p).setError(::std::current_exception());
				}
			}
		);

		return future;
	}

	template <AsyncContinuation<T> F>
	auto then(F &&cont) &&
	{
		using U = typename ::utils::detail::result_<F &, T>::value_type;

		auto [future, promise] = futures::makeContractVia<U>(getExecutor());

		auto redirect = futures::Callback<U>(
			[p = ::std::move(promise)]
			(::utils::result<U> &&r) mutable noexcept {
				if constexpr (
					::std::is_nothrow_move_constructible_v<::utils::result<U>>
				) {
					::std::move(p).setResult(::std::move(r));
				} else try {
					::std::move(p).setResult(::std::move(r));
				} catch (...) {
					::std::move(p).setError(::std::current_exception());
				}
			}
		);

		::std::move(*this).subscribe(
			[f = ::std::forward(cont), c = ::std::move(redirect)]
			(Result &&res) mutable noexcept {
				if (res.has_error()) {
					c(::std::move(res).error());
					return;
				}

				if constexpr (::std::is_nothrow_invocable_v<F &, T>) {
					::std::invoke(f, ::std::move(res).value())
						.subscribe(::std::move(c));
				} else try {
					::std::invoke(f, ::std::move(res).value())
						.subscribe(::std::move(c));
				} catch (...) {
					c(::std::current_exception());
				}
			}
		);

		return future;
	}

	template <ErrorHandler<T> F>
	auto recover(F &&hand) &&
	{
		auto [future, promise] = futures::makeContractVia<T>(getExecutor());

		::std::move(*this).subscribe(
			[f = ::std::forward<F>(hand), p = ::std::move(promise)]
			(Result &&res) mutable noexcept {
				if constexpr (
					::std::is_nothrow_invocable_v<F &, ::utils::error> &&
					::std::is_nothrow_move_constructible_v<Result>
				) {
					::std::move(p).setResult(::std::move(res).or_else(f));
				} else try {
					::std::move(p).setResult(::std::move(res).or_else(f));
				} catch (...) {
					::std::move(p).setError(::std::current_exception());
				}
			}
		);

		return future;
	}
};

template <::utils::suitable_for_result T>
Contract<T> makeContractVia(executors::IExecutor &where)
{
	auto state = detail::SharedState<T>::create(where);
	return {Future<T>(state), Promise<T>(state)};
}

template <typename T>
Contract<T> makeContract()
{
	return futures::makeContractVia<T>(executors::getInlineExecutor());
}

template <typename F>
auto asyncVia(executors::IExecutor &where, F &&what)
	requires (::utils::is_invocable_v<F &>)
{
	using T = ::utils::detail::result_<F &>;

	auto [future, promise] = futures::makeContract<T>();

	// exception handling (return futures::failure(exception_ptr))
	executors::execute(
		where,
		[f = ::std::forward<F>(what), p = ::std::move(promise)]
		() mutable noexcept {
			if constexpr (
				::std::is_nothrow_invocable_v<F &> &&
				::std::is_nothrow_move_constructible_v<::utils::result<T>>
			) {
				::std::move(p).setResult(::std::invoke(f));
			} else try {
				::std::move(p).setResult(::std::invoke(f));
			} catch (...) {
				::std::move(p).setError(::std::current_exception());
			}
		}
	);

	return future;
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_CORE_FUTURE_H_ */
