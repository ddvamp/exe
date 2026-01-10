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
#include <exe/future/fun/core/operator.hpp>
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
  explicit InvokeWith(Args &&...args) : args_(::std::move(args)...) {}

  template <typename T>
  requires (::std::is_invocable_v<T &&, Args &&...>)
  auto Apply(Future<T> f) && {
    return ::std::move(f) | future::Map([args = ::std::move(args_)]
                                        (T &&t) mutable -> decltype(auto) {
      return ::std::apply(::std::move(t), ::std::move(args));
    });
  }
};

} // namespace pipe

template <typename ...Ts>
inline auto InvokeWith(Ts ...ts) {
  return pipe::InvokeWith(::std::move(ts)...);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_INVOKE_WITH_HPP_INCLUDED_ */
