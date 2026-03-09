//
// unify_void.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

// [TODO]: ?Move to util

#ifndef DDVAMP_EXE_UNIFY_VOID_HPP_INCLUDED_
#define DDVAMP_EXE_UNIFY_VOID_HPP_INCLUDED_ 1

#include <exe/unit.hpp>

#include <util/type_traits.hpp>

namespace exe::trait {

namespace detail {

template <typename T>
struct UnifyVoidImpl {
  using Type = T;
};

template <typename T>
requires (::std::is_void_v<T>)
struct UnifyVoidImpl<T> {
  using Type = ::util::cv_like_t<T, Unit>;
};

} // namespace detail

template <typename T>
using UnifyVoid = detail::UnifyVoidImpl<T>::Type;

} // namespace exe::trait

#endif /* DDVAMP_EXE_UNIFY_VOID_HPP_INCLUDED_ */
