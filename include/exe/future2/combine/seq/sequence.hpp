//
// sequence.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_COMBINE_SEQ_SEQUENCE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_COMBINE_SEQ_SEQUENCE_HPP_INCLUDED_ 1

#include <exe/future2/thunk.hpp>
#include <exe/future2/thunk/seq/sequence.hpp>

#include <util/concepts.hpp>

namespace exe::future {

template <::util::rvalue_deduced ...Ts>
inline Thunk<thunk::Sequence<Ts...>> Sequence(Ts &&...ts) {
  return Thunk(thunk::Sequence(::std::move(ts)...));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_COMBINE_SEQ_SEQUENCE_HPP_INCLUDED_ */
