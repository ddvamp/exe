//
// eager_state_impl.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_DETAIL_EAGER_EAGER_STATE_IMPL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_DETAIL_EAGER_EAGER_STATE_IMPL_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/detail/eager/rendezvous_state.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/thunk/seq/box.hpp>

#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <type_traits>

namespace exe::future::detail {

// [TODO]: Use util::refer
template <typename T, typename Derived>
class EagerStateImpl : private RendezvousState<T>,
                       public cancel::CancelSource,
                       public core::IBoxedThunk<T>,
                       private cancel::HandlerBase {
 private:
  using Base = RendezvousState<T>;

  ::std::atomic_size_t ref_cnt_ = 2;

  // To guarantee the expected implementation
  static_assert(::std::atomic_size_t::is_always_lock_free);

 protected:
  using Base::SetState;

  void SetValue(T &&t) {
    Base::SetValue(::std::move(t));
    ReleaseHandlers();
  }

  void SetCancel() {
    UTIL_ASSERT(CancelRequested(), "Upstream Cancel without request");
    Base::SetCancel();
    Release();
  }

  void SetConsumer(IConsumer<T> &c) {
    if (Base::SetConsumer(c)) {
      Release();
      return;
    }

    c.CancelSource().AddHandler(*this);
  }

 private:
  void RequestCancel() {
    cancel::CancelSource::RequestCancel();
    Release();
  }

  void ReleaseHandlers() {
    // [TODO]: Implement this
    RequestCancel();
  }

  // cancel::HandlerBase
  void OnCancelRequested() && noexcept override {
    RequestCancel();
  }

  // IBoxedThunk
  void Drop() && noexcept override {
    RequestCancel();
  }

  /* Lifetime management */

  void Release() {
    auto const remains = ref_cnt_.fetch_sub(1, ::std::memory_order_release);
    if (remains > 1) [[likely]] {
      return;
    }

    UTIL_ASSERT(remains != 0, "Unexpected Release() call");
    ::util::sync_with_release_sequences(ref_cnt_);
    DestroySelf();
  }

  void DestroySelf() {
    static_assert(requires { static_cast<Derived *>(this); } &&
                  ::std::is_final_v<Derived>);
    delete static_cast<Derived *>(this);
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_DETAIL_EAGER_EAGER_STATE_IMPL_HPP_INCLUDED_ */
