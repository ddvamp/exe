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
#include <cstddef>
#include <optional>
#include <tuple>
#include <utility>

namespace exe::future {

namespace detail {

template <concepts::FutureValue V>
class [[nodiscard]] SharedState final : public core::IBoxedThunk<V>,
                                        public cancel::CancelSource,
                                        private cancel::HandlerBase {
 private:
  ::std::optional<V> result_;
  ::std::optional<State> state_;
  IConsumer<V> *consumer_;
  ::concurrency::Rendezvous rendezvous_;

  ::std::atomic_size_t ref_cnt_ = 3;

  // To guarantee the expected implementation
  static_assert(::std::atomic_size_t::is_always_lock_free);

 public:
  /* Producer */

  void SetValue(V &&v) noexcept {
    result_.emplace(::std::move(v));
    TryConsume();
    RequestCancel(); // ReleaseHandlers();
  }

  void SetCancel() noexcept {
    TryConsume();
    Release();
  }

  /* IBoxedThunk / Consumer */

  void Start(IConsumer<V> &c, Scheduler &s) && noexcept override {
    c.CancelSource().AddHandler(*this);

    state_.emplace(State{s});
    consumer_ = &c;
    TryConsume();
    Release();
  }

  void Drop() && noexcept override {
    RequestCancel(2);
  }

 private:
  void RequestCancel(::std::size_t cnt = 1) noexcept {
    cancel::CancelSource::RequestCancel();
    Release(cnt);
  }

  // cancel::IHandler
  void OnCancelRequested() && noexcept override {
    RequestCancel();
  }

  void TryConsume() noexcept {
    if (!rendezvous_.Arrive()) {
      return;
    }

    if (result_.has_value()) [[likely]] {
      ::std::move(*consumer_).Continue(*::std::move(result_), *state_);
    } else {
      ::std::move(*consumer_).Cancel(*state_);
    }
  }

  void Release(::std::size_t cnt = 1) noexcept {
    if (ref_cnt_.fetch_sub(cnt, ::std::memory_order_release) == cnt) {
      ::util::sync_with_release_sequences(ref_cnt_);
      DestroySelf();
    }
  }

  void DestroySelf() noexcept {
    delete this;
  }

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
