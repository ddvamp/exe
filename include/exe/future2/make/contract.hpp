//
// contract.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MAKE_CONTRACT_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MAKE_CONTRACT_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/thunk/seq/box.hpp>

#include <concurrency/rendezvous.hpp>
#include <util/abort.hpp>
#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <optional>
#include <tuple>
#include <utility>

namespace exe::future {

namespace detail {

template <typename T>
class RendezvousState {
 private:
  struct Consumer {
    IConsumer<T> &c;
    State s;
  };

  ::std::optional<T> producer_;
  ::std::optional<Consumer> consumer_;
  ::concurrency::Rendezvous rendezvous_;

 protected:
  /* Producer */

  void SetValue(T &&v) {
    producer_.emplace(::std::move(v));
    TryContinue();
  }

  void SetCancel() {
    // producer_ is empty
    TryCancel();
  }

  /* Consumer */

  bool SetConsumer(IConsumer<T> &c, Scheduler &s) {
    consumer_.emplace(c, State{s});
    return TryContinue();
  }

 private:
  bool TryContinue() {
    if (!rendezvous_.Arrive()) {
      return false;
    }

    auto &v = *producer_;
    auto &[c, s] = *consumer_;
    ::std::move(c).Continue(::std::move(v), s);
    return true;
  }

  void TryCancel() {
    if (rendezvous_.Arrive()) {
      auto &[c, s] = *consumer_;
      ::std::move(c).Cancel(s);
    }
  }
};

template <concepts::FutureValue V>
class [[nodiscard]] SharedState final : private RendezvousState<V>,
                                        public cancel::CancelSource,
                                        public core::IBoxedThunk<V>,
                                        private cancel::HandlerBase {
 private:
  using Base = RendezvousState<V>;

  ::std::atomic_size_t ref_cnt_ = 2;

  // To guarantee the expected implementation
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  /* Promise */

  void SetValue(V &&v) noexcept {
    Base::SetValue(::std::move(v));
    RequestCancel(); // ReleaseHandlers();
  }

  void SetCancel() noexcept {
    Base::SetCancel();
    Release();
  }

 private:
  /* IBoxedThunk */

  void Start(IConsumer<V> &c, Scheduler &s) && noexcept override {
    if (Base::SetConsumer(c, s)) {
      Release();
      return;
    }

    c.CancelSource().AddHandler(*this);
  }

  void Drop() && noexcept override {
    RequestCancel();
  }

  // cancel::HandlerBase
  void OnCancelRequested() && noexcept override {
    RequestCancel();
  }

  void RequestCancel() noexcept {
    cancel::CancelSource::RequestCancel();
    Release();
  }

  /* Lifetime management */

  void Release() noexcept {
    auto const remains = ref_cnt_.fetch_sub(1, ::std::memory_order_release);
    if (remains > 1) [[likely]] {
      return;
    }

    UTIL_ASSERT(remains != 0, "Unexpected Release() call");
    ::util::sync_with_release_sequences(ref_cnt_);
    DestroySelf();
  }

  void DestroySelf() noexcept {
    delete this;
  }

  // Private
  ~SharedState() = default;
};

} // namespace detail

template <concepts::FutureValue>
class Promise;

template <concepts::FutureValue V>
::std::tuple<Thunk<thunk::Box<V>>, Promise<V>> Contract();

template <concepts::FutureValue V>
class [[nodiscard]] Promise {
  friend ::std::tuple<Thunk<thunk::Box<V>>, Promise<V>> Contract<V>();

 private:
  using State = detail::SharedState<V>;

  // [TODO]: StateHolder/PtrHolder/EagerHolder
  State *state_;

 public:
  ~Promise() {
    if (!IsValid()) [[likely]] {
      return;
    }

    // Autocancel
    if (state_->CancelRequested()) [[likely]] {
      state_->SetCancel();
    }

    UTIL_ABORT("Promise is lost");
  }

  Promise(Promise const &) = delete;
  void operator= (Promise const &) = delete;

  // Move-out only
  Promise(Promise &&that) noexcept : state_(that.Release()) {}
  void operator= (Promise &&) = delete;

 public:
  // [TODO]: ?State
  void Set(V v) && noexcept {
    ReleaseChecked()->SetValue(::std::move(v));
  }

  // [TODO]: ?State
  void Cancel() && noexcept {
    auto state = ReleaseChecked();
    UTIL_ASSERT(state->CancelRequested(), "Upstream Cancel without request");
    state->SetCancel();
  }

  // Cancellation

  cancel::CancelSource &CancelSource() const noexcept {
    return *static_cast<cancel::CancelSource *>(GetChecked());
  }

 private:
  explicit Promise(State *s) noexcept : state_(s) {}

  bool IsValid() const noexcept {
    return state_;
  }

  State *GetChecked() const noexcept {
    Check();
    return state_;
  }

  [[nodiscard]] State *Release() noexcept {
    return ::std::exchange(state_, nullptr);
  }

  [[nodiscard]] State *ReleaseChecked() noexcept {
    Check();
    return Release();
  }

  void Check() const noexcept {
    UTIL_ASSERT(IsValid(), "Invalid Promise");
  }
};

template <concepts::FutureValue V>
::std::tuple<Thunk<thunk::Box<V>>, Promise<V>> Contract() {
  auto state = ::new detail::SharedState<V>;
  return {Thunk(thunk::Box<V>(state)), Promise(state)};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_CONTRACT_HPP_INCLUDED_ */
