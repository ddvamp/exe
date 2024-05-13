// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_ 1

#include <utility>

#include <context/context.hpp>
#include <utils/memory/view.hpp>

#include "body.hpp"

namespace exe::fiber {

// Basis for suspended execution of fibers
class Coroutine : public ::context::ITrampoline {
 private:
  Body body_;
  ::context::ExecutionContext context_;
  bool is_completed_ = false;

 public:
  Coroutine(Body &&body, ::utils::memory_view stack) noexcept
      : body_(::std::move(body))
      , context_(stack, this) {}

  [[nodiscard]] bool IsCompleted() const noexcept {
    return is_completed_;
  }

  /* External call functions */

  void Resume() noexcept {
    context_.SwitchToSaved();
  }

  /* Internal call functions */

  void Suspend() noexcept {
    context_.SwitchToSaved();
  }

  [[noreturn]] void Cancel() noexcept {
    is_completed_ = true;
    context_.ExitToSaved();
  }

 private:
  [[noreturn]] void DoRun() noexcept override {
    ::std::move(body_)();
    Cancel();
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_ */
