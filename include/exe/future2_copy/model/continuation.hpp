//
// continuation.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_CONTINUATION_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_CONTINUATION_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/model/state.hpp>

#include <concepts>

namespace exe::future::concepts {

template <typename C, typename V>
concept Continuation = requires (C c, V v, State s) {
  { ::std::move(c).Continue(::std::move(v), s) } noexcept
      -> ::std::same_as<void>;
  { ::std::move(c).Cancel(s) } noexcept -> ::std::same_as<void>;
  { c.CancelSource() } noexcept -> ::std::same_as<cancel::CancelSource &>; // [TODO]: ?concept CancelSource
};

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_CONTINUATION_HPP_INCLUDED_ */
