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
#include <exe/future2/core/ptr.hpp>
#include <exe/future2/core/release_ptr.hpp>
#include <exe/future2/detail/eager/eager_state_impl.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/result/result.hpp>

#include <tuple>
#include <utility>

namespace exe::future {

namespace detail {

template <typename T>
class PromiseState final : public EagerStateImpl<T, PromiseState<T>> {
 private:
  using Base = PromiseState::EagerStateImpl;

 public:
  using Base::SetValue;
  using Base::SetCancel;

 private:
  // IBoxedThunk
  void Start(IConsumer<T> &c, Scheduler &s) && noexcept override {
    Base::SetState(State{s});
    Base::SetConsumer(c);
  }
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <concepts::FutureValue>
class Promise;

template <typename T>
::std::tuple<Thunk<thunk::Box<T>>, Promise<T>> Contract();

template <concepts::FutureValue V>
class [[nodiscard]] Promise
    : private core::ReleasePtr<detail::PromiseState<V>> {
  friend ::std::tuple<Thunk<thunk::Box<V>>, Promise<V>> Contract<V>();

 public:
  void Set(V v) && noexcept {
    this->ReleaseChecked()->SetValue(::std::move(v));
  }

  void Cancel() && noexcept {
    this->ReleaseChecked()->SetCancel();
  }

  // Cancellation
  cancel::CancelSource &CancelSource() const & noexcept {
    return *this->GetChecked();
  }

 protected:
  using Promise::ReleasePtr::ReleasePtr;
};

template <typename T>
::std::tuple<Thunk<thunk::Box<T>>, Promise<T>> Contract() {
  auto state = ::new detail::PromiseState<T>;
  return {Thunk(thunk::Box<T>(state)), Promise<T>(state)};
}

////////////////////////////////////////////////////////////////////////////////

// [TODO]: ?Better
struct BrokenPromise {};

template <concepts::FutureValue>
class SafePromise;

template <typename T>
::std::tuple<Thunk<thunk::Box<Result<T, BrokenPromise>>>, SafePromise<T>> SafeContract();

template <concepts::FutureValue V>
class [[nodiscard]] SafePromise
    : private core::Ptr<detail::PromiseState<Result<V, BrokenPromise>>> {
  friend ::std::tuple<Thunk<thunk::Box<Result<V, BrokenPromise>>>, SafePromise<V>> SafeContract<V>();

 private:
  using Base = SafePromise::Ptr;

 public:
  ~SafePromise() {
    if (this->IsValid()) [[unlikely]] {
      this->Get()->SetValue(result::Err<V, BrokenPromise>({}));
    }
  }

  SafePromise(SafePromise const &) = delete;
  void operator= (SafePromise const &) = delete;

  // Move-out only
  SafePromise(SafePromise &&that) noexcept : Base(that.Release()) {}
  void operator= (SafePromise &&) = delete;

 public:
  void Set(V v) && noexcept {
    this->ReleaseChecked()
        ->SetValue(result::Ok<V, BrokenPromise>(::std::move(v)));
  }

  void Cancel() && noexcept {
    this->ReleaseChecked()->SetCancel();
  }

  // Cancellation
  cancel::CancelSource &CancelSource() const & noexcept {
    return *this->GetChecked();
  }

 protected:
  using Base::Base;
};

template <typename T>
::std::tuple<Thunk<thunk::Box<Result<T, BrokenPromise>>>, SafePromise<T>> SafeContract() {
  auto state = ::new detail::PromiseState<Result<T, BrokenPromise>>;
  return {Thunk(thunk::Box<Result<T, BrokenPromise>>(state)), SafePromise<T>(state)};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_CONTRACT_HPP_INCLUDED_ */
