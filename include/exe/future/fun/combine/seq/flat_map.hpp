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

#include <exe/future/fun/core/mapper.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/make/contract.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/trait/value_of.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <type_traits>
#include <utility>

namespace exe::future {

namespace pipe {

template <core::concepts::Mapper Fn>
class [[nodiscard]] FlatMap : private core::Operator {
 private:
  Fn fn_;

 public:
  explicit FlatMap(Fn &&fn) noexcept : fn_(::std::move(fn)) {}

  template <typename T>
  requires (::std::is_invocable_v<Fn &&, T &&>)
  Future<trait::ValueOf<::std::invoke_result_t<Fn &&, T &&>>> Apply(Future<T> f) && {
    using U = trait::ValueOf<::std::invoke_result_t<Fn &&, T &&>>;

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
          SetCallback(SetScheduler(::std::move(fn)(*::std::move(res)),
                                   runtime::GetInline()), ::std::move(cb));
        } catch (...) {
          ::std::move(cb)(result::Err<U>(CurrentError()));
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
inline auto FlatMap(Fn fn) noexcept {
  return pipe::FlatMap(core::Mapper(::std::move(fn)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLAT_MAP_HPP_INCLUDED_ */
