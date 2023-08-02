// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_UTILITY_H_
#define DDV_UTILS_UTILITY_H_ 1

#include <cstddef>
#include <utility>

#include "utils/macro.h"
#include "utils/type_traits.h"

namespace utils {

// Type placeholder
struct unit_t {};

inline constexpr unit_t unit{};


////////////////////////////////////////////////////////////////////////////////


template <typename ...>
struct types_list_t {};

template <typename ...Ts>
inline constexpr types_list_t<Ts...> types_list{};

struct deduce_type_t {};

inline constexpr deduce_type_t deduce_type{};

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


////////////////////////////////////////////////////////////////////////////////


/**
 *	@brief		Convert a rvalue to an lvalue.
 *	@param t	A thing of arbitrary type.
 *	@return		The parameter explicitly converted to an lvalue-reference.
 *
 *	This function can be used to convert an prvalue to an lvalue.
 *	In this case temporary materialization occurs.
 */
template <typename T>
[[nodiscard]] inline constexpr T &temporary(T &&t) noexcept
	requires (!::std::is_lvalue_reference_v<T>)
{
	return static_cast<T &>(t);
}


////////////////////////////////////////////////////////////////////////////////


// Creates decayed copy of an object
// Deprecated since C++23 (auto{} syntax)
template <typename T>
[[nodiscard]] inline constexpr ::std::decay_t<T> decay_copy(T &&t)
	noexcept (::std::is_nothrow_convertible_v<T, ::std::decay_t<T>>)
{
	return ::std::forward<T>(t);
}


////////////////////////////////////////////////////////////////////////////////


// simple utility for combining lambdas
template <typename ...Ts>
struct overloaded : Ts... { using Ts::operator()...; };


////////////////////////////////////////////////////////////////////////////////


// Class that allows to delay the execution of a functor for
// initializing an object until it is actually created
//
// During initialization, type deduction should not take place,
// since the type of the builder itself will be deduced
template <typename Fn>
requires (
	::std::is_class_v<Fn> &&
	::std::is_move_constructible_v<Fn> &&
	::std::is_destructible_v<Fn> &&
	::std::is_invocable_v<Fn &> &&
	!::std::is_void_v<::std::invoke_result_t<Fn &>>
)
class builder {
private:
	UTILS_NO_UNIQUE_ADDRESS Fn fn_;

public:
	explicit builder(Fn const &fn)
		noexcept (::std::is_nothrow_copy_constructible_v<Fn>)
		requires (::std::is_copy_constructible_v<Fn>)
		: fn_(fn)
	{}

	explicit builder(Fn &&fn)
		noexcept (::std::is_nothrow_move_constructible_v<Fn>)
		: fn_(::std::move(fn))
	{}

	[[nodiscard]] operator decltype(auto) ()
		noexcept (::std::is_nothrow_invocable_v<Fn &>)
	{
		return fn_();
	}
};


////////////////////////////////////////////////////////////////////////////////


template <::std::size_t I, typename T>
struct tuple_val {
	UTILS_NO_UNIQUE_ADDRESS T val_;
};

template <typename, typename ...Ts>
struct tuple_impl;

template <::std::size_t ...Is, typename ...Ts>
struct tuple_impl<::std::index_sequence<Is...>, Ts...>
	: tuple_val<Is, Ts>... {};

// for a guaranteed order of construction of elements
// TODO: add structured bindings support
template <typename ...Ts>
struct tuple : tuple_impl<::std::index_sequence_for<Ts...>, Ts...> {};



template <::std::size_t I, typename ...Ts>
[[nodiscard]] auto &get(tuple<Ts...> &t) noexcept
	requires (I < sizeof...(Ts))
{
	using Ttype = tuple_val<I, pack_element_t<I, Ts...>>;
	return static_cast<Ttype &>(t).val_;
}


////////////////////////////////////////////////////////////////////////////////

// to check bool values without laziness/unnecessary branches

template <typename ...Ts>
[[nodiscard]] constexpr bool all_of(Ts &&...ts) noexcept
	requires (all_true_v<::std::is_constructible_v<bool, Ts>...>)
{
	return (1 & ... & static_cast<int>(static_cast<bool>(::std::forward<Ts>(ts)))) == 1;
}

template <typename ...Ts>
[[nodiscard]] constexpr bool any_of(Ts &&...ts) noexcept
	requires (all_true_v<::std::is_constructible_v<bool, Ts>...>)
{
	return (0 | ... | static_cast<int>(static_cast<bool>(::std::forward<Ts>(ts)))) == 1;
}

template <typename ...Ts>
[[nodiscard]] constexpr bool none_of(Ts &&...ts) noexcept
	requires (all_true_v<::std::is_constructible_v<bool, Ts>...>)
{
	return (0 | ... | static_cast<int>(static_cast<bool>(::std::forward<Ts>(ts)))) == 0;
}

} // namespace utils

#endif /* DDV_UTILS_UTILITY_H_ */
