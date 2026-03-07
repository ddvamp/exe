//
// proxy.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CORE_PROXY_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CORE_PROXY_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/model/state.hpp>

#include <cstddef>
#include <utility>

namespace exe::future::core {

template <typename Raw>
struct Proxy {
  Raw &raw_;

  void Continue(auto &&v, State s) && noexcept {
    ::std::move(raw_).Continue(::std::move(v), s);
  }

  void Cancel(State s) && noexcept {
    ::std::move(raw_).Cancel(s);
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return raw_.CancelSource();
  }
};

template <typename Raw, ::std::size_t I>
struct Redirect {
  Raw &raw_;

  void Continue(auto &&v, State s) && noexcept {
    ::std::move(raw_).template Continue<I>(::std::move(v), s);
  }

  void Cancel(State s) && noexcept {
    ::std::move(raw_).template Cancel<I>(s);
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return raw_.CancelSource();
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_CORE_PROXY_HPP_INCLUDED_ */
