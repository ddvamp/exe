//
// first.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_FIRST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_FIRST_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/detail/first_state.hpp>
#include <exe/future/fun/trait/value_of.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <util/type_traits.hpp>

#include <functional>
#include <utility>

namespace exe::future {

namespace detail {

class FirstImpl : private core::Operator {
 public:
  template <typename ...Ts>
  static SemiFuture<Ts...[0]> Apply(SemiFuture<Ts> ...fs) {
    auto const state = ::new FirstState<Ts...[0]>(sizeof...(Ts));

    (..., SetCallback(SetScheduler(::std::move(fs), runtime::GetInline()),
                      ::std::ref(*state)));

    return state->ToFuture();
  }
};

} // namespace detail

template <concepts::Future ...Fs>
requires ((sizeof...(Fs) > 1) && ::util::is_all_same_v<trait::ValueOf<Fs>...>)
inline auto First(Fs ...fs) {
  return detail::FirstImpl::Apply(::std::move(fs)...);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_FIRST_HPP_INCLUDED_ */
