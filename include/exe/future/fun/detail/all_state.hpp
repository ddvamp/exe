//
// all_state.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_DETAIL_ALL_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_DETAIL_ALL_STATE_HPP_INCLUDED_ 1

#include <exe/future/fun/core/future_factory.hpp>
#include <exe/future/fun/core/future_state.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/future_fwd.hpp>

#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

namespace exe::future::detail {

template <typename>
class AllState;

template <typename ...Ts>
class AllState<::std::tuple<Ts...>> final
    : private core::FutureState<::std::tuple<Ts...>>
    , private core::FutureFactory {
 private:
  using T = ::std::tuple<Ts...>;

  ::std::tuple<::std::optional<Ts>...> vals_;
  Error error_ = nullptr;
  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t live_;
  ::std::atomic_size_t ref_cnt_;

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  AllState() noexcept : live_(sizeof...(Ts)), ref_cnt_(sizeof...(Ts)) {}

  SemiFuture<T> ToFuture() noexcept {
    return MakeSemiFuture(this);
  }

  template <::std::size_t I>
  void SetValue(Ts...[I] &&val) noexcept {
    ::std::get<I>(vals_).emplace(::std::move(val));

    if (live_.fetch_sub(1, ::std::memory_order_release) == 1) [[unlikely]] {
      ::util::SyncWithReleaseSequences(live_);
      this->TrySchedule();
    } else {
      Release();
    }
  }

  void SetError(Error &&err) noexcept {
    if (first_.exchange(false, ::std::memory_order_relaxed)) [[unlikely]] {
      error_ = ::std::move(err);
      this->TrySchedule();
    } else {
      Release();
    }
  }

 private:
  // TaskBase
  void Run() && noexcept override {
    (*::std::move(this->callback_))(MakeResult());
    Release();
  }

  void Release() noexcept {
    if (ref_cnt_.fetch_sub(1, ::std::memory_order_release) == 1) [[unlikely]] {
      ::util::SyncWithReleaseSequences(ref_cnt_);
      delete this;
    }
  }

  [[nodiscard]] Result<T> MakeResult() noexcept {
    if (error_) {
      return result::Err<T>(::std::move(error_));
    }

    return [&]<::std::size_t ...Idx>(::std::index_sequence<Idx...>) noexcept {
      return result::Ok(T(*::std::get<Idx>(::std::move(vals_))...));
    }(::std::index_sequence_for<Ts...>{});
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_FUN_DETAIL_ALL_STATE_HPP_INCLUDED_ */
