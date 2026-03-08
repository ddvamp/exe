//
// value.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MAKE_VALUE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MAKE_VALUE_HPP_INCLUDED_ 1

#include <exe/future2/make/ready.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/type/future.hpp>
#include <exe/result/trait/result.hpp>

#include <utility>

namespace exe::future {

template <concepts::FutureValue V>
requires (!exe::trait::Result<V>)
concepts::Future<V> auto Value(V v) {
  return Ready(::std::move(v));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_VALUE_HPP_INCLUDED_ */
