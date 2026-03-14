//
// first.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_COMBINE_CONCUR_FIRST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_COMBINE_CONCUR_FIRST_HPP_INCLUDED_ 1

#include <exe/future2/thunk.hpp>
#include <exe/future2/thunk/concur/first.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/future2/type/future.hpp>

namespace exe::future {

template <concepts::SomeFuture ...Fs>
concepts::SomeFuture auto First(Fs &&...fs) {
  return Thunk(thunk::Box(::new thunk::First(::std::move(fs)...)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_COMBINE_CONCUR_FIRST_HPP_INCLUDED_ */
