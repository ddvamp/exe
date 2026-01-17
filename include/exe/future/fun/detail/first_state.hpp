//
// first_state.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_DETAIL_FIRST_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_DETAIL_FIRST_STATE_HPP_INCLUDED_ 1

#include <exe/future/fun/core/future_factory.hpp>
#include <exe/future/fun/core/future_state.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/future_fwd.hpp>

#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstddef>
#include <utility>

namespace exe::future::detail {

template <typename T>
class FirstState final : private core::FutureState<T>
                       , private core::FutureFactory {
 private:
  Result<T> result_;
  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t live_;
  ::std::atomic_size_t ref_cnt_;

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  explicit FirstState(::std::size_t sz) noexcept : live_(sz), ref_cnt_(sz) {}

  SemiFuture<T> ToFuture() noexcept {
    return MakeSemiFuture(this);
  }

  void operator() (Result<T> &&res) noexcept {
    if (res.has_value() ? IsFirstOk() : IsLastError()) [[unlikely]] {
      result_ = ::std::move(res);
      this->TrySchedule();
    } else {
      Release();
    }
  }

 private:
  // TaskBase
  void Run() && noexcept override {
    (*::std::move(this->callback_))(::std::move(result_));
    Release();
  }

  void Release() noexcept {
    if (ref_cnt_.fetch_sub(1, ::std::memory_order_release) == 1) [[unlikely]] {
      ::util::SyncWithReleaseSequences(ref_cnt_);
      delete this;
    }
  }

  [[nodiscard]] bool IsFirstOk() noexcept {
    return first_.exchange(false, ::std::memory_order_relaxed);
  }

  [[nodiscard]] bool IsLastError() noexcept {
    return live_.fetch_sub(1, ::std::memory_order_relaxed) == 1;
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_FUN_DETAIL_FIRST_STATE_HPP_INCLUDED_ */
