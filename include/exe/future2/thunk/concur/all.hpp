//
// all.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_CONCUR_ALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_CONCUR_ALL_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/core/proxy.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/future2/trait/computation.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/type/future.hpp>

#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

namespace exe::future::thunk {

namespace detail {

template <typename Thunk, typename All, ::std::size_t I>
struct AllBuilder {
  All &a;
  Thunk &p;

  using R = trait::Computation<Thunk, core::Redirect<All, I>>;

  operator R () && noexcept {
    return R({a}, ::std::move(p));
  }
};

} // namespace detail

template <concepts::SomeFuture ...Fs>
requires (sizeof...(Fs) > 1)
class [[nodiscard]] All final
    : public core::IBoxedThunk<::std::tuple<trait::ValueOf<Fs>...>>,
      private cancel::CancelSource,
      private cancel::HandlerBase {
 private:
  using ValueType = ::std::tuple<trait::ValueOf<Fs>...>;

  template <::std::size_t I>
  using ValueOf = trait::ValueOf<Fs...[I]>;

  template <typename Producer, ::std::size_t I>
  using Comp = trait::Computation<Producer, core::Redirect<All, I>>;

  template <::std::size_t ...Is>
  static ::std::tuple<Comp<Fs, Is>...> MakeComps(
      ::std::index_sequence<Is...>, All &all, Fs &&...fs) noexcept {
    return {detail::AllBuilder<Fs, All, Is>{all, fs}...};
  }

  using ISeq = ::std::index_sequence_for<Fs...>;

  using Comps = decltype(MakeComps(::std::declval<ISeq>(),
                                   ::std::declval<All &>(),
                                   ::std::declval<Fs>()...));

  IConsumer<ValueType> *cons_;
  Scheduler *sched_;
  ::std::tuple<::std::optional<trait::ValueOf<Fs>>...> results_;

  // [TODO]: ?Replace with inheritance
  Comps comps_;

  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t live_ = sizeof...(Fs);
  ::std::atomic_size_t ref_cnt_ = 1 + sizeof...(Fs);

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  All(Fs &&...fs) noexcept
      : comps_(MakeComps(ISeq{}, *this, ::std::move(fs)...)) {}

  /* IBoxedThunk */

  void Start(IConsumer<ValueType> &c, Scheduler &s) && noexcept final {
    cons_ = &c;
    sched_ = &s;

    cons_->CancelSource().AddHandler(*this);

    auto &[...comps] = comps_;
    (..., ::std::move(comps).Start(s));
  }

  void Drop() && noexcept final {
    DestroySelf();
  }

  /* Continuation */

  template <::std::size_t I>
  void Continue(ValueOf<I> &&v, State) && noexcept {
    ::std::get<I>(results_).emplace(::std::move(v));

    if (TryLate()) [[unlikely]] {
      ::std::move(*cons_).Continue(MakeResult(), {.sched = *sched_});
      RequestCancel();
    }

    Release();
  }

  template <::std::size_t>
  void Cancel(State s) && noexcept {
    CancelImpl(s);
  }

  cancel::CancelSource &CancelSource() noexcept {
    return *this;
  }

 private:
  ValueType MakeResult() noexcept {
    ::util::sync_with_release_sequences(live_);

    auto &[...opt_vals] = results_;
    return {*::std::move(opt_vals)...};
  }

  void CancelImpl(State) noexcept {
    UTIL_ASSERT(CancelRequested(), "Upstream Cancel without request");

    if (TryEarly()) [[unlikely]] {
      ::std::move(*cons_).Cancel({.sched = *sched_});
    }

    Release();
  }

  // cancel::IHandler
  void OnCancelRequested() && noexcept override {
    RequestCancel();
    Release();
  }

  [[nodiscard]] bool TryEarly() noexcept {
    return first_.exchange(false, ::std::memory_order_relaxed);
  }

  [[nodiscard]] bool TryLate() noexcept {
    return live_.fetch_sub(1, ::std::memory_order_release) == 1;
  }

  void Release() noexcept {
    if (ref_cnt_.fetch_sub(1, ::std::memory_order_release) == 1) {
      ::util::sync_with_release_sequences(ref_cnt_);
      DestroySelf();
    }
  }

  void DestroySelf() noexcept {
    delete this;
  }
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_CONCUR_ALL_HPP_INCLUDED_ */
