// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_ 1

#include <context/context.hpp>
#include <util/memory/view.hpp>

#include "body.hpp"

namespace exe::fiber {

// Basis for suspended execution of fibers
class Coroutine : public ::context::ITrampoline {
 public:
  enum class Status {
    active,
    inactive,
    completed
  };

 private:
  Body body_;
  ::context::ExecutionContext context_;
  Status status_ = Status::inactive;

 public:
  Coroutine(Body &&body, ::util::memory_view stack) noexcept;

  [[nodiscard]] bool IsCompleted() const noexcept;

  /* External call functions */

  void Resume() noexcept;

  /* Internal call functions */

  void Suspend() noexcept;

  [[noreturn]] void Cancel() noexcept;

 private:
  [[noreturn]] void DoRun() noexcept override;

  void ChangeStatus([[maybe_unused]] Status const from,
                    Status const to) noexcept;
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_COROUTINE_HPP_INCLUDED_ */
