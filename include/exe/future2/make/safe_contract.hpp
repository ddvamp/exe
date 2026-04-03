//
// safe_contract.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MAKE_SAFE_CONTRACT_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MAKE_SAFE_CONTRACT_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/core/ptr.hpp>
#include <exe/future2/detail/eager/promise_state.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/result/result.hpp>

#include <tuple>
#include <utility>

namespace exe::future {

// [TODO]: ?Better
struct BrokenPromise {};

namespace detail {

template <typename T>
using SafePromiseState = PromiseState<Result<T, BrokenPromise>>;

} // namespace detail

template <concepts::FutureValue>
auto SafeContract();

template <concepts::FutureValue V>
class [[nodiscard]] SafePromise
    : private core::Ptr<detail::SafePromiseState<V>> {
  friend auto SafeContract<V>();

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

template <concepts::FutureValue T>
auto SafeContract() {
  auto state = ::new detail::SafePromiseState<T>;
  return ::std::tuple{Thunk(thunk::Box(state)), SafePromise<T>(state)};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_SAFE_CONTRACT_HPP_INCLUDED_ */
