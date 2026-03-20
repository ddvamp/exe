//
// box.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_COMBINE_SEQ_BOX_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_COMBINE_SEQ_BOX_HPP_INCLUDED_ 1

#include <exe/future2/pipe.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/type/future.hpp>

namespace exe::future {

namespace pipe {

struct Box {
  template <concepts::SomeFuture F>
  concepts::Future<trait::ValueOf<F>> auto Pipe(F &&f) {
    return Thunk(thunk::Box(::std::move(f)));
  }
};

} // namespace pipe

inline auto Box() {
  return pipe::Box{};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_COMBINE_SEQ_BOX_HPP_INCLUDED_ */
