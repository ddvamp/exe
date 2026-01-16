//
// hold_state.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_HOLD_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_HOLD_STATE_HPP_INCLUDED_ 1

#include <util/debug.hpp>

#include <utility>

namespace exe::future::core {

template <typename State>
class HoldState {
 private:
  State *state_;

 protected:
  ~HoldState() {
    UTIL_ASSERT(!HasState(),
                "HoldState is not used at the time of destruction");
  }

  HoldState(HoldState const &) = delete;
  void operator= (HoldState const &) = delete;

  HoldState(HoldState &&that) noexcept : state_(that.Release()) {}
  void operator= (HoldState &&) = delete;

 protected:
  explicit HoldState(State *state) noexcept : state_(state) {}

  [[nodiscard("Pure")]] bool HasState() const noexcept {
    return state_;
  }

  /* Unchecked */

  [[nodiscard("Pure")]] State *GetState() noexcept {
    return state_;
  }

  [[nodiscard("Pure")]] State const *GetState() const noexcept {
    return state_;
  }

  [[nodiscard]] State *Release() noexcept {
    return ::std::exchange(state_, nullptr);
  }

  /* Checked */

  [[nodiscard("Pure")]] State *GetStateChecked() noexcept {
    UTIL_DEBUG_RUN(CheckState);
    return GetState();
  }

  [[nodiscard("Pure")]] State const *GetStateChecked() const noexcept {
    UTIL_DEBUG_RUN(CheckState);
    return GetState();
  }

  [[nodiscard]] State *ReleaseChecked() noexcept {
    UTIL_DEBUG_RUN(CheckState);
    return Release();
  }

 private:
  [[maybe_unused]] void CheckState() const noexcept {
    UTIL_ASSERT(HasState(),
                "HoldState does not hold state when it is expected");
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_HOLD_STATE_HPP_INCLUDED_ */
