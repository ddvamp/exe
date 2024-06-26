// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_
#define DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_ 1

#include <type_traits>

#include "exe/futures/fun/make/contract/fwd.h"
#include "exe/futures/fun/mutator/fwd.h"
#include "exe/futures/fun/state/shared_state.h"

namespace exe::futures {

template <typename>
inline constexpr bool is_noncv_future_v = false;

template <typename T>
struct is_noncv_future : ::std::bool_constant<is_noncv_future_v<T>> {};



template <typename T>
inline constexpr bool is_future_v = is_noncv_future_v<::std::remove_cv_t<T>>;

template <typename T>
struct is_future : ::std::bool_constant<is_future_v<T>> {};



namespace concepts {

template <typename F>
concept Future = is_noncv_future_v<F>;

template <typename F, typename T>
concept FutureOf = Future<F> && ::std::is_same_v<typename F::value_type, T>;

template <typename F>
concept Event = FutureOf<F, void>;

} // namespace concepts



template <concepts::Future F>
inline static constexpr bool has_executor_v = false;


////////////////////////////////////////////////////////////////////////////////


// Class for representing future value
// Future is moveable value type
template <typename>
class Future;



template <typename T>
class [[nodiscard]] SemiFuture : protected detail::HoldState<T> {
	friend Contract<T>;
	friend detail::Mutator;

protected:
	using Base = detail::HoldState<T>;

	using Base::Base;

	using without_executor = SemiFuture;
	using with_executor = Future<T>;

public:
	using Base::value_type;

	using Base::Callback;
};

template <typename T>
inline constexpr bool is_noncv_future_v<SemiFuture<T>> = true;



template <typename T>
class [[nodiscard]] Future : public SemiFuture<T> {
	friend detail::Mutator;

protected:
	using SemiFuture<T>::SemiFuture;
};

template <typename T>
inline constexpr bool is_noncv_future_v<Future<T>> = true;

template <typename T>
inline static constexpr bool has_executor_v<Future<T>> = true;

////////////////////////////////////////////////////////////////////////////////

// TODO: ReadySemiFuture / ReadyFuture

////////////////////////////////////////////////////////////////////////////////

template <concepts::Future>
class FutureHolder;



template <concepts::Future F>
class [[nodiscard]] SemiFutureHolder {
	friend detail::Mutator;

protected:
	F &raw;

	explicit SemiFutureHolder(F &f) noexcept
		: raw(f)
	{}

	using without_executor = SemiFutureHolder;
	using with_executor = FutureHolder<F>;

public:
	using value_type = F::value_type;

	using Callback = F::Callback;
};

template <typename T>
inline constexpr bool is_noncv_future_v<SemiFutureHolder<T>> = true;



template <concepts::Future F>
class [[nodiscard]] FutureHolder : public SemiFutureHolder<F> {
	friend detail::Mutator;

protected:
	using SemiFutureHolder<F>::SemiFutureHolder;
};

template <typename T>
inline constexpr bool is_noncv_future_v<FutureHolder<T>> = true;

template <typename T>
inline static constexpr bool has_executor_v<FutureHolder<T>> = true;

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_ */
