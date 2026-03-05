//
// has_cancel_source.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_DETAIL_HAS_CANCEL_SOURCE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_DETAIL_HAS_CANCEL_SOURCE_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>

#include <concepts>

namespace exe::future::detail {

template <typename Consumer>
concept HasCancelSource = requires (Consumer c) {
  { c.CancelSource() } noexcept -> ::std::same_as<cancel::CancelSource &>;
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_DETAIL_HAS_CANCEL_SOURCE_HPP_INCLUDED_ */
