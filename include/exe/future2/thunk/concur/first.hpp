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
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/future2/trait/computation.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/type/future.hpp>

#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <tuple>
#include <utility>

namespace exe::future::thunk {

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

template <concepts::SomeFuture F, concepts::Future<trait::ValueOf<F>> ...Fs>
requires (sizeof...(Fs) != 0)
class [[nodiscard]] First final
    : public core::IBoxedThunk<trait::ValueOf<F>>,
      private cancel::CancelSource,
      private cancel::HandlerBase {
 private:
  using ValueType = trait::ValueOf<F>;

  template <typename Producer>
  using Comp = trait::Computation<Producer, core::Proxy<First>>;

  IConsumer<ValueType> *cons_;
  Scheduler *sched_;

  // [TODO]: ?Replace with inheritance
  ::std::tuple<Comp<F>, Comp<Fs>...> comps_;

  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t ref_cnt_ = 2 + sizeof...(Fs);

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  First(F &&f, Fs &&...fs) noexcept
      : comps_{detail::FirstBuilder{*this, f},
               detail::FirstBuilder{*this, fs}...} {}

  /* IBoxedThunk */

  void Start(IConsumer<ValueType> &c, Scheduler &s) && noexcept override {
    cons_ = &c;
    sched_ = &s;

    cons_->CancelSource().AddHandler(*this);

    auto &[...comps] = comps_;
    (..., ::std::move(comps).Start(s));
  }

  void Drop() && noexcept override {
    DestroySelf();
  }

  /* Continuation */

  void Continue(ValueType &&v, State) && noexcept {
    if (TryEarly()) [[unlikely]] {
      ::std::move(*cons_).Continue(::std::move(v), {.sched = *sched_});
      RequestCancel();
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

#endif /* DDVAMP_EXE_FUTURE_THUNK_CONCUR_FIRST_HPP_INCLUDED_ */
