//
// all.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_COMBINE_CONCUR_ALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_COMBINE_CONCUR_ALL_HPP_INCLUDED_ 1

#include <exe/future2/thunk.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/thunk/concur/all.hpp>
#include <exe/future2/thunk/seq/box.hpp>

namespace exe::future {

template <concepts::Thunk ...Ts>
auto All(Ts &&...ts) {
  return Thunk(thunk::Box(::new thunk::All(::std::move(ts)...)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_COMBINE_CONCUR_ALL_HPP_INCLUDED_ */
