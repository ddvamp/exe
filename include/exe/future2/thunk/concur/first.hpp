//
// first.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_CONCUR_FIRST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_CONCUR_FIRST_HPP_INCLUDED_ 1

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
#include <tuple>
#include <utility>

namespace exe::future {

namespace thunk {

namespace detail {

template <typename Thunk, typename First>
struct FirstBuilder {
  First &f;
  Thunk &p;

  using R = trait::Computation<Thunk, core::Proxy<First>>;

  operator R () && noexcept {
    return R({f}, ::std::move(p));
  }
};

} // namespace detail

template <concepts::Thunk ...Producers>
requires (sizeof...(Producers) > 1 &&
          ::util::is_all_same_v<trait::ValueOf<Producers>...>)
class [[nodiscard]] First final
    : public core::IBoxedThunk<trait::ValueOf<Producers...[0]>>,
      private cancel::CancelSource,
      private cancel::HandlerBase {
 private:
  using ValueType = trait::ValueOf<Producers...[0]>;

  template <typename Producer>
  using Comp = trait::Computation<Producer, core::Proxy<First>>;

  IConsumer<ValueType> *cons_;
  Scheduler *sched_;

  // [TODO]: ?Replace with inheritance
  ::std::tuple<Comp<Producers>...> comps_;

  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t live_ = sizeof...(Producers);
  ::std::atomic_size_t ref_cnt_ = 1 + sizeof...(Producers);

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);
  static_assert(::std::atomic<cancel::HandlerBase *>::is_always_lock_free);

 public:
  First(Producers &&...prods) noexcept
      : comps_{detail::FirstBuilder{*this, prods}...} {}

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

  void Continue(ValueType &&v, State) && noexcept {
    if (TryEarly()) [[unlikely]] {
      // [TODO]: ?Order
      RequestCancel();
      ::std::move(*cons_).Continue(::std::move(v), {.sched = *sched_});
    }

    Release();
  }

  void Cancel(State) && noexcept {
    UTIL_ASSERT(CancelRequested(), "Upstream Cancel without request");

    if (TryEarly()) [[unlikely]] {
      ::std::move(*cons_).Cancel({.sched = *sched_});
    }

    Release();
  }

  cancel::CancelSource &CancelSource() noexcept {
    return *this;
  }

 private:
  // cancel::IHandler
  void OnCancelRequested() && noexcept override {
    RequestCancel();
    Release();
  }

  [[nodiscard]] bool TryEarly() noexcept {
    return first_.exchange(false, ::std::memory_order_relaxed);
  }

  [[nodiscard]] bool TryLate() noexcept {
    return live_.fetch_sub(1, ::std::memory_order_relaxed) == 1;
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
auto First(Producers &&...prods) {
  return Thunk(thunk::Box(::new thunk::First(::std::move(prods)...)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_CONCUR_FIRST_HPP_INCLUDED_ */
