// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_STATE_CALLBACK_H_
#define DDV_EXE_FUTURES_FUN_STATE_CALLBACK_H_ 1

#include <cstddef>
#include <functional>
#include <utility>

#include "result/result.h"

#include "utils/type_traits.h"

namespace exe::futures {

// Instead of synchronously waiting for future value
//
// User must take care of passing arguments, returning values,
// and handling exceptions himself
template <typename T>
using Callback =
	::std::move_only_function<void(::utils::result<T> &&) noexcept>;



namespace detail {

template <::std::size_t I, typename T>
struct TupleVal {
	[[no_unique_address]] T val_;
};



template <typename, typename ...Ts>
struct TupleImpl;

template <::std::size_t ...Is, typename ...Ts>
struct TupleImpl<::std::index_sequence<Is...>, Ts...> : TupleVal<Is, Ts>... {};



// for a guaranteed order of construction of elements
template <typename ...Ts>
struct Tuple : TupleImpl<::std::index_sequence_for<Ts...>, Ts...> {};



template <::std::size_t I, typename ...Ts>
[[nodiscard]] auto &get(Tuple<Ts...> &t) noexcept
	requires (I < sizeof...(Ts))
{
	using Ttype = TupleVal<I, ::utils::pack_element_t<I, Ts...>>;
	return static_cast<Ttype &>(t).val_;
}

////////////////////////////////////////////////////////////////////////////////

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
	[[no_unique_address]] Fn fn_;
	[[no_unique_address]] Tuple<Args...> args_;

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
			::std::invoke(fn_, res, detail::get<I>(args_)...);
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

// TODO: move to utils
template <typename ...>
struct types_list_t {};

template <typename ...Ts>
inline constexpr types_list_t<Ts...> types_list{};

struct deduce_type_t {};

template <typename T, typename>
struct deduce {
	using type = T;
};

template <typename U>
struct deduce<deduce_type_t, U> {
	using type = U;
};

template <typename T, typename U>
using deduce_t = deduce<T, U>::type;

template <typename T, typename ...Args, typename ...Ts>
[[nodiscard]] auto makeCallback(types_list_t<Args...>, Ts &&...ts)
	requires (sizeof...(Args) == sizeof...(Ts))
{
	return Callback<T>(
		::std::in_place_type<detail::Callback<
			T,
			deduce_t<Args, ::std::remove_cvref_t<Ts>>...
		>>,
		::std::forward<Ts>(ts)...
	);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_STATE_CALLBACK_H_ */
