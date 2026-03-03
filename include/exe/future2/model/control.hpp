//
// control.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_CONTROL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_CONTROL_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/concept/async_safe.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/state.hpp>

#include <concepts>

namespace exe::future::concepts {

template <typename C, typename V, typename R>
concept Control = requires (C c, V v, State s) {
  requires AsyncSafe<C>;

  { c.Data() } noexcept -> ::std::same_as<R &>;

  { ::std::move(c).Return(::std::move(v), s) } noexcept
      -> ::std::same_as<void>;
  { ::std::move(c).Cancel(s) } noexcept -> ::std::same_as<void>;
  { c.CancelSource() } noexcept -> ::std::same_as<cancel::CancelSource &>;
};

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_CONTROL_HPP_INCLUDED_ */
