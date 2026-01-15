//
// callback.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_DETAIL_CALLBACK_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_DETAIL_CALLBACK_HPP_INCLUDED_ 1

#include <exe/future/fun/result/result.hpp>

#include <functional>

namespace exe::future::detail {

/**
 *  Instead of synchronously waiting for a future value
 *
 *  The user must take care of passing arguments, returning values,
 *  and handling exceptions himself
 *
 *  Must be initialized by a callable with non-throwing destructor
 */
template <typename T>
using Callback = ::std::move_only_function<void(Result<T> &&) && noexcept>;

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_FUN_DETAIL_CALLBACK_HPP_INCLUDED_ */
