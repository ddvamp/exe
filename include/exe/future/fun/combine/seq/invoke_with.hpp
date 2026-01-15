//
// invoke_with.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_INVOKE_WITH_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_INVOKE_WITH_HPP_INCLUDED_ 1

#include <exe/future/fun/combine/seq/map.hpp>
#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/core/concept/adapt_call.hpp>
#include <exe/future/fun/core/trait/adapt_call.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/future_fwd.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace exe::future {

namespace pipe {

template <typename ...Args>
class [[nodiscard]] InvokeWith : private core::Operator {
 private:
  ::std::tuple<Args...> args_;

 public:
  explicit InvokeWith(Args ...args) : args_(::std::move(args)...) {}

  template <typename Fn>
  requires (core::concepts::AdaptOutputInvocable<Fn, Args...> &&
            concepts::FutureValue<core::trait::AdaptOutputResult<Fn, Args...>>)
  Future<core::trait::AdaptOutputResult<Fn, Args...>> Apply(Future<Fn> f) && {
    auto cb = [args = ::std::move(args_)](Fn &&fn) mutable -> decltype(auto) {
      return ::std::apply(::std::move(fn), ::std::move(args));
    };

    return ::std::move(f) | future::Map(::std::move(cb));
  }
};

} // namespace pipe

template <typename ...Args>
requires (... && ::std::is_nothrow_destructible_v<::std::decay_t<Args>>)
inline auto InvokeWith(Args &&...args) {
  return pipe::InvokeWith(::std::forward<Args>(args)...);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_INVOKE_WITH_HPP_INCLUDED_ */
