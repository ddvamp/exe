//
// recover.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_RECOVER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_RECOVER_HPP_INCLUDED_ 1

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
class [[nodiscard]] Recover : private core::Operator {
 private:
  Fn fn_;

 public:
  explicit Recover(Fn fn) : fn_(::std::move(fn)) {}

  template <typename T>
  requires (core::concepts::AdaptOutputInvocable<Fn, Error> &&
            ::std::is_same_v<T, core::trait::AdaptOutputResult<Fn, Error>>)
  Future<T> Apply(Future<T> f) && {
    auto [nf, p] = core::Contract<T>();

    auto cb = [p = ::std::move(p), fn = ::std::move(fn_)]
              (Result<T> &&res) mutable noexcept {
      if (res.has_value()) {
        ::std::move(p).SetValue(*::std::move(res));
      } else {
        try {
          ::std::move(p).SetValue(core::AdaptOutput(::std::move(fn),
                                                    ::std::move(res).error()));
        } catch (...) {
          ::std::move(p).SetError(CurrentError());
        }
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
inline auto Recover(Fn &&fn) {
  return pipe::Recover(::std::forward<Fn>(fn));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_RECOVER_HPP_INCLUDED_ */
