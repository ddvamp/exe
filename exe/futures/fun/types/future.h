// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_
#define DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_ 1

#include "exe/futures/fun/make/contract/fwd.h"
#include "exe/futures/fun/mutator/fwd.h"
#include "exe/futures/fun/state/shared_state.h"

#include "utils/type_traits.h"

namespace exe::futures {

template <typename T>
class [[nodiscard]] SemiFuture : protected detail::HoldState<T> {
	friend Contract<T>;
	friend detail::Mutator;

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
	friend detail::Mutator;

protected:
	using SemiFuture<T>::SemiFuture;
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline constexpr bool is_noncv_future_v = false;

template <typename T>
inline constexpr bool is_noncv_future_v<SemiFuture<T>> = true;

template <typename T>
inline constexpr bool is_noncv_future_v<Future<T>> = true;

template <typename T>
struct is_noncv_future : ::std::bool_constant<is_noncv_future_v<T>> {};



template <typename T>
inline constexpr bool is_future_v = is_noncv_future_v<::std::remove_cv_t<T>>;

template <typename T>
struct is_future : ::std::bool_constant<is_future_v<T>> {};

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_ */
