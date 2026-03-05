//
// has_continue.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_DETAIL_HAS_CONTINUE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_DETAIL_HAS_CONTINUE_HPP_INCLUDED_ 1

#include <exe/future2/model/state.hpp>

#include <concepts>
#include <utility>

namespace exe::future::detail {

template <typename Consumer, typename InputType>
concept HasContinue = requires (Consumer c, InputType v, State s) {
  { ::std::move(c).Continue(::std::move(v), s) } noexcept
      -> ::std::same_as<void>;
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_DETAIL_HAS_CONTINUE_HPP_INCLUDED_ */
