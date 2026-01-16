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
#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/trait/value_of.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/future/fun/type/promise.hpp>
#include <exe/runtime/inline.hpp>

#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

namespace exe::future {

namespace detail {

class AllImpl : private core::Operator {
 private:
  template <typename ...Ts>
  struct State {
    using T = ::std::tuple<Ts...>;

    Promise<T> p_;
    ::std::tuple<::std::optional<Ts>...> vals_;
    ::std::atomic_bool first_ = true;
    ::std::atomic_size_t live_;
    ::std::atomic_size_t ref_cnt_;

    // To guarantee the expected implementation
    static_assert(::std::atomic_bool::is_always_lock_free);
    static_assert(::std::atomic_size_t::is_always_lock_free);

    explicit State(Promise<T> p) noexcept
        : p_(::std::move(p))
        , live_(sizeof...(Ts))
        , ref_cnt_(sizeof...(Ts)) {}

    template <::std::size_t I>
    void SetValue(Ts...[I] &&val) noexcept {
      ::std::get<I>(vals_).emplace(::std::move(val));

      if (live_.fetch_sub(1, ::std::memory_order_release) == 1) [[unlikely]] {
        ::util::SyncWithReleaseSequences(live_);
        ::std::move(p_).SetValue(MakeTuple());
      }

      Release();
    }

    void SetError(Error &&err) noexcept {
      if (first_.exchange(false, ::std::memory_order_relaxed)) [[unlikely]] {
        ::std::move(p_).SetError(::std::move(err));
      }

      Release();
    }

    void Release() noexcept {
      if (ref_cnt_.fetch_sub(1, ::std::memory_order_release) == 1) [[unlikely]] {
        ::util::SyncWithReleaseSequences(ref_cnt_);
        delete this;
      }
    }

    [[nodiscard]] T MakeTuple() noexcept {
      return [&]<::std::size_t ...Idx>(::std::index_sequence<Idx...>) noexcept {
        return ::std::tuple(*::std::get<Idx>(::std::move(vals_))...);
      }(::std::index_sequence_for<Ts...>{});
    }
  };

 public:
  template <typename ...Fs>
  static SemiFuture<::std::tuple<trait::ValueOf<Fs>...>> Apply(Fs ...fs) {
    using T = ::std::tuple<trait::ValueOf<Fs>...>;

    auto [f, p] = core::Contract<T>();

    auto const state = ::new State(::std::move(p));

    [&]<::std::size_t ...Idx>(::std::index_sequence<Idx...>) {
      (..., SetCallback(SetScheduler(::std::move(fs), runtime::GetInline()),
                        [state](Result<trait::ValueOf<Fs>> &&res) noexcept {
                          if (res.has_value()) {
                            state->template SetValue<Idx>(*::std::move(res));
                          } else {
                            state->SetError(::std::move(res).error());
                          }
                        }));
    }(::std::index_sequence_for<Fs...>{});

    return ::std::move(f);
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
