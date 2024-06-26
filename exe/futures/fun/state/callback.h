// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_STATE_CALLBACK_H_
#define DDV_EXE_FUTURES_FUN_STATE_CALLBACK_H_ 1

#include <cstddef>
#include <functional>

#include "result/result.h"

#include "utils/macro.h"
#include "utils/type_traits.h"
#include "utils/utility.h"

namespace exe::futures {

// Instead of synchronously waiting for future value
//
// User must take care of passing arguments, returning values,
// and handling exceptions himself
template <typename T>
using Callback =
	::std::move_only_function<void(::utils::result<T> &&) noexcept>;



namespace detail {

template <typename T, typename Fn, typename ...Args>
requires (
	::std::is_nothrow_invocable_v<Fn &, ::utils::result<T> &, Args &...> &&
	::utils::all_true_v<
		::std::is_nothrow_destructible_v<Fn>,
		::std::is_nothrow_destructible_v<Args>...
	>
)
class Callback {
private:
	UTILS_NO_UNIQUE_ADDRESS Fn fn_;
	UTILS_NO_UNIQUE_ADDRESS ::utils::tuple<Args...> args_;

public:
	template <typename TFn, typename ...TArgs>
	explicit Callback(TFn &&fn, TArgs &&...args)
		requires (
			::utils::all_true_v<
				::std::is_constructible_v<Fn, TFn>,
				::std::is_constructible_v<Args, TArgs>...
			>
		)
		: fn_(::std::forward<TFn>(fn))
		, args_{::std::forward<TArgs>(args)...}
	{}

	void operator() (::utils::result<T> &&res) noexcept
	{
		[&, this]<::std::size_t ...I>(::std::index_sequence<I...>) noexcept {
			::std::invoke(fn_, res, ::utils::get<I>(args_)...);
		}(::std::index_sequence_for<Args...>{});
	}
};

} // namespace detail

template <typename T, typename ...Ts>
[[nodiscard]] auto makeCallback(Ts &&...ts)
{
	return Callback<T>(
		::std::in_place_type<
			detail::Callback<T, ::std::remove_cvref_t<Ts>...>
		>,
		::std::forward<Ts>(ts)...
	);
}

template <typename T, typename ...Args, typename ...Ts>
[[nodiscard]] auto makeCallback(::utils::types_list_t<Args...>, Ts &&...ts)
	requires (sizeof...(Args) == sizeof...(Ts))
{
	return Callback<T>(
		::std::in_place_type<detail::Callback<
			T,
			::utils::deduce_t<Args, ::std::remove_cvref_t<Ts>>...
		>>,
		::std::forward<Ts>(ts)...
	);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_STATE_CALLBACK_H_ */
