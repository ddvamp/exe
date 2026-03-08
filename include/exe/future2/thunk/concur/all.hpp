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
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/future2/trait/computation.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <util/concepts.hpp>
#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

namespace exe::future {

namespace thunk {

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

template <typename ...Thunks>
using AllResult = ::std::tuple<trait::ValueOf<Thunks>...>;

} // namespace detail

template <concepts::Thunk ...Producers>
requires (sizeof...(Producers) > 1)
class [[nodiscard]] All final
    : public core::IBoxedThunk<detail::AllResult<Producers...>>,
      private cancel::CancelSource,
      private cancel::HandlerBase {
 private:
  using ValueType = detail::AllResult<Producers...>;

  template <::std::size_t I>
  using ValueOf = trait::ValueOf<Producers...[I]>;

  template <typename Producer, ::std::size_t I>
  using Comp = trait::Computation<Producer, core::Redirect<All, I>>;

  template <::std::size_t ...Is>
  static ::std::tuple<Comp<Producers, Is>...> MakeComps(
      ::std::index_sequence<Is...>, All &all, Producers &&...ps) noexcept {
    return {detail::AllBuilder<Producers, All, Is>{all, ps}...};
  }

  using ISeq = ::std::index_sequence_for<Producers...>;

  using Comps = decltype(MakeComps(::std::declval<ISeq>(),
                                   ::std::declval<All &>(),
                                   ::std::declval<Producers>()...));

  IConsumer<ValueType> *cons_;
  Scheduler *sched_;
  ::std::tuple<::std::optional<trait::ValueOf<Producers>>...> results_;

  // [TODO]: ?Replace with inheritance
  Comps comps_;

  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t live_ = sizeof...(Producers);
  ::std::atomic_size_t ref_cnt_ = 1 + sizeof...(Producers);

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  All(Producers &&...prods) noexcept
      : comps_(MakeComps(ISeq{}, *this, ::std::move(prods)...)) {}

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

} // namespace thunk

// [TODO]: Move to future/combine/
template <::util::rvalue_deduced ...Producers>
auto All(Producers &&...prods) {
  return Thunk(thunk::Box(::new thunk::All(::std::move(prods)...)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_CONCUR_ALL_HPP_INCLUDED_ */
