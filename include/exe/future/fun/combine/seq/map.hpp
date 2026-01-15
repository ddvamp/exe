//
// map.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/core/adapt_call.hpp>
#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/core/concept/adapt_call.hpp>
#include <exe/future/fun/core/trait/adapt_call.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/future_fwd.hpp>

#include <type_traits>
#include <utility>

namespace exe::future {

namespace pipe {

template <typename Fn>
class [[nodiscard]] Map : private core::Operator {
 private:
  Fn fn_;

 public:
  explicit Map(Fn fn) : fn_(::std::move(fn)) {}

  template <typename T>
  requires (core::concepts::AdaptCallInvocable<Fn, T> &&
            concepts::FutureValue<core::trait::AdaptCallResult<Fn, T>>)
  Future<core::trait::AdaptCallResult<Fn, T>> Apply(Future<T> f) && {
    using U = core::trait::AdaptCallResult<Fn, T>;

    auto [nf, p] = core::Contract<U>();

    auto cb = [p = ::std::move(p), fn = ::std::move(fn_)]
              (Result<T> &&res) mutable noexcept {
      if (res.has_value()) {
        try {
          ::std::move(p).SetValue(core::AdaptCall(::std::move(fn),
                                                  *::std::move(res)));
        } catch (...) {
          ::std::move(p).SetError(CurrentError());
        }
      } else {
        ::std::move(p).SetError(::std::move(res).error());
      }
    };

    auto &where = GetScheduler(f);

    SetCallback(::std::move(f), ::std::move(cb));

    return SetScheduler(::std::move(nf), where);
  }
};

} // namespace pipe

template <typename Fn>
requires (::std::is_nothrow_destructible_v<::std::decay_t<Fn>>)
inline auto Map(Fn &&fn) {
  return pipe::Map(::std::forward<Fn>(fn));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_ */
