//
// all.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_ALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_ALL_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/detail/all_state.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <cstddef>
#include <tuple>
#include <utility>

namespace exe::future {

namespace detail {

class AllImpl : private core::Operator {
 public:
  template <typename ...Ts>
  static SemiFuture<::std::tuple<Ts...>> Apply(SemiFuture<Ts> ...fs) {
    auto const state = ::new AllState<::std::tuple<Ts...>>;

    [&]<::std::size_t ...Idx>(::std::index_sequence<Idx...>) {
      (..., SetCallback(SetScheduler(::std::move(fs), runtime::GetInline()),
                        [state](Result<Ts> &&res) noexcept {
                          if (res.has_value()) {
                            state->template SetValue<Idx>(*::std::move(res));
                          } else {
                            state->SetError(::std::move(res).error());
                          }
                        }));
    }(::std::index_sequence_for<Ts...>{});

    return state->ToFuture();
  }
};

} // namespace detail

template <concepts::Future ...Fs>
requires (sizeof...(Fs) > 1)
inline auto All(Fs ...fs) {
  return detail::AllImpl::Apply(::std::move(fs)...);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_ALL_HPP_INCLUDED_ */
