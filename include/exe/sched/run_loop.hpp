// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_RUN_LOOP_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_RUN_LOOP_HPP_INCLUDED_ 1

#include "task/scheduler.hpp"

#include <cstddef>

namespace exe::sched {

class RunLoop final : public task::ISafeScheduler {
 private:
   // TODO: Intrusive queue
   task::TaskBase *head_ = nullptr;
   task::TaskBase *tail_;

 public:
   ~RunLoop();

   RunLoop(RunLoop const &) = delete;
   void operator= (RunLoop const &) = delete;

   RunLoop(RunLoop &&) = delete;
   void operator= (RunLoop &&) = delete;

 public:
   RunLoop() = default;

   void Submit(task::TaskBase *) noexcept override;

   ::std::size_t RunAtMost(::std::size_t const limit) noexcept;

   bool RunNext() noexcept {
     return RunAtMost(1) != 0;
   }

   ::std::size_t Run() noexcept {
     return RunAtMost(::std::size_t{} - 1);
   }

   [[nodiscard]] bool IsEmpty() const noexcept {
     return !head_;
   }

 private:
   [[nodiscard]] task::TaskBase *Pop() noexcept;
};

} // namespace exe::sched

#endif /* DDVAMP_EXE_SCHED_RUN_LOOP_HPP_INCLUDED_ */
