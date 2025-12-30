//
// coroutine.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_ 1

#include <exe/fiber/core/body.hpp>

#include <context/context.hpp>
#include <context/trampoline.hpp>
#include <util/memory/view.hpp>

namespace exe::fiber {

/* Basis for suspended execution of fibers */
class Coroutine : public ::context::ITrampoline {
 public:
  enum class Status {
    kInactive,
    kActive,
    kCompleted
  };

  using enum Status;

 private:
  Body body_;
  ::context::ExecutionContext context_;
  Status status_ = kInactive;

 public:
  Coroutine(Body &&body, ::util::memory_view stack) noexcept;

  [[nodiscard]] bool IsCompleted() const noexcept;

  /* External call functions */

  void Resume() noexcept;

  /* Internal call functions */

  void Suspend() noexcept;

  [[noreturn]] void Complete() noexcept;

 private:
  [[noreturn]] void DoRun() noexcept override;

  void ChangeStatus(Status from, Status to) noexcept;
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_ */
