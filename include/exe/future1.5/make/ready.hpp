//
// ready.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MAKE_READY_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MAKE_READY_HPP_INCLUDED_ 1

#include <exe/future2/thunk.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/thunk/make/ready.hpp>
#include <exe/future2/type/future.hpp>

namespace exe::future {

// [TODO]: Pipe

template <concepts::FutureValue V>
concepts::Future<V> auto Ready(V v) {
  return Thunk(thunk::Ready(::std::move(v)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_READY_HPP_INCLUDED_ */
