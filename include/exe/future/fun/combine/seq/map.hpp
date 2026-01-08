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

#include <exe/future/fun/make/contract.hpp>
#include <exe/future/fun/operator/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/error.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/future/fun/type/result.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace exe::future {

namespace pipe {

template <::std::destructible Fn>
class [[nodiscard]] Map : public Operator {
 private:
  Fn fn_;

 public:
  explicit Map(Fn fn) : fn_(::std::move(fn)) {}

  template <typename T>
  Future<::std::invoke_result_t<Fn &&, T &&>> Apply(Future<T> f) && {
    auto [nf, p] = Contract<::std::invoke_result_t<Fn &&, T &&>>();

    auto cb = [p = ::std::move(p), fn = ::std::move(fn_)]
              (Result<T> &&res) mutable noexcept {
      if (res.has_value()) {
        try {
          ::std::move(p).SetValue(::std::move(fn)(*::std::move(res)));
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
inline pipe::Map<Fn> Map(Fn fn) {
  return pipe::Map(::std::move(fn));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_ */
