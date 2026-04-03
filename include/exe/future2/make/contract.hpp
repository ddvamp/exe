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
#include <exe/future2/thunk.hpp>
#include <exe/future2/core/release_ptr.hpp>
#include <exe/future2/detail/eager/promise_state.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/thunk/seq/box.hpp>

#include <tuple>
#include <utility>

namespace exe::future {

template <concepts::FutureValue>
auto Contract();

template <concepts::FutureValue V>
class [[nodiscard]] Promise
    : private core::ReleasePtr<detail::PromiseState<V>> {
  friend auto Contract<V>();

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

template <concepts::FutureValue T>
auto Contract() {
  auto state = ::new detail::PromiseState<T>;
  return ::std::tuple{Thunk(thunk::Box(state)), Promise<T>(state)};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_CONTRACT_HPP_INCLUDED_ */
