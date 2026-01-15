//
// context.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONTEXT_CONTEXT_HPP_INCLUDED_
#define DDVAMP_CONTEXT_CONTEXT_HPP_INCLUDED_ 1

#include <context/exceptions_context.hpp>
#include <context/machine_context.hpp>

#include <util/debug/unreachable.hpp>
#include <util/memory/view.hpp>

namespace context {

/* Execution context = machine context (registers state) + exceptions context */
class ExecutionContext {
 private:
  MachineContext machine_ctx_;
  ExceptionsContext exceptions_ctx_;

 public:
  ~ExecutionContext() = default;

  ExecutionContext(ExecutionContext const &) = delete;
  void operator= (ExecutionContext const &) = delete;

  ExecutionContext(ExecutionContext &&) = delete;
  void operator= (ExecutionContext &&) = delete;

 public:
  // Empty slot for current context saving
  ExecutionContext() = default;

  // Initializes new context with stack using trampoline
  ExecutionContext(::util::memory_view stack,
                   ITrampoline *trampoline) noexcept {
    machine_ctx_.Setup(stack, trampoline);
  }

  // Save current context in this and reset target context
  // (this and target are allowed to be aliased)
  void SwitchTo(ExecutionContext &target) noexcept {
    // Order is significant
    exceptions_ctx_.SwitchTo(target.exceptions_ctx_);
    machine_ctx_.SwitchTo(target.machine_ctx_);
  }

  void SwitchToSaved() noexcept {
    SwitchTo(*this);
  }

  // Last context switch
  [[noreturn]] void ExitTo(ExecutionContext &target) noexcept {
    SwitchTo(target);
    UTIL_UNREACHABLE("Resuming a completed ExecutionContext");
  }

  [[noreturn]] void ExitToSaved() noexcept {
    ExitTo(*this);
  }
};

} // namespace context

#endif /* DDVAMP_CONTEXT_CONTEXT_HPP_INCLUDED_ */
