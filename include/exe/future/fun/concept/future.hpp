//
// future.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CONCEPT_FUTURE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CONCEPT_FUTURE_HPP_INCLUDED_ 1

#include <exe/future/fun/type/future_fwd.hpp>

namespace exe::future::concepts {

namespace detail {

template <typename>
inline constexpr bool IsFutureImpl = false;

template <typename T>
inline constexpr bool IsFutureImpl<SemiFuture<T>> = true;

template <typename T>
inline constexpr bool IsFutureImpl<Future<T>> = true;

} // namespace detail

template <typename T>
concept Future = detail::IsFutureImpl<T>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_FUN_CONCEPT_FUTURE_HPP_INCLUDED_ */
