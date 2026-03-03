//
// async_safe.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CONCEPT_ASYNC_SAFE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CONCEPT_ASYNC_SAFE_HPP_INCLUDED_ 1

#include <type_traits>

namespace exe::future::concepts {

template <typename T>
concept AsyncSafe =
    ::std::is_nothrow_destructible_v<T> &&       // destroy context
    ::std::is_nothrow_move_constructible_v<T> && // direct-init context
    ::std::is_nothrow_convertible_v<T &&, T>;    // copy-init context

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_CONCEPT_ASYNC_SAFE_HPP_INCLUDED_ */
