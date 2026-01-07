//
// flat_map.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLAT_MAP_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLAT_MAP_HPP_INCLUDED_ 1

#include <exe/future/fun/make/contract.hpp>
#include <exe/future/fun/operator/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/trait/value_of.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/future/fun/type/result.hpp>
#include <exe/runtime/inline.hpp>

#include <concepts>
#include <exception>
#include <type_traits>
#include <utility>

namespace exe::future {

namespace pipe {

template <::std::destructible Fn>
class [[nodiscard]] FlatMap : public Operator {
 private:
  Fn fn_;

 public:
  explicit FlatMap(Fn fn) : fn_(::std::move(fn)) {}

  template <typename T>
  Future<ValueOf<::std::invoke_result_t<Fn &&, T &&>>> Apply(Future<T> f) && {
    using U = ValueOf<::std::invoke_result_t<Fn &&, T &&>>;

    auto [nf, p] = Contract<U>();

    auto cb = [p = ::std::move(p), fn = ::std::move(fn_)]
              (Result<T> &&res) mutable noexcept {
      if (res.has_value()) {
        auto cb = [p = ::std::move(p)](Result<U> &&res) mutable noexcept {
          if (p.IsValid()) [[likely]] {
            ::std::move(p).SetResult(::std::move(res));
          }
        };

        try {
          auto f = SetScheduler(::std::move(fn)(*::std::move(res)),
                                runtime::GetInline());
          SetCallback<U>(::std::move(f), ::std::move(cb));
        } catch (...) {
          ::std::move(cb)(result::Err<U>(::std::current_exception()));
        }
      } else {
        ::std::move(p).SetError(::std::move(res).error());
      }
    };

    auto &where = GetScheduler(f);

    SetCallback<T>(::std::move(f), ::std::move(cb));

    return SetScheduler(::std::move(nf), where);
  }
};

} // namespace pipe

template <typename Fn>
inline pipe::FlatMap<Fn> FlatMap(Fn fn) {
  return pipe::FlatMap(::std::move<Fn>(fn));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLAT_MAP_HPP_INCLUDED_ */
