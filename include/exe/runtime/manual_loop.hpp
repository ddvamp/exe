//
// manual_loop.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_MANUAL_LOOP_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_MANUAL_LOOP_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/intrusive/queue.hpp>

#include <cstddef>

namespace exe::runtime {

/* Single-thread task queue */
class ManualLoop final : public task::ISafeScheduler {
 private:
  ::util::intrusive_queue<task::TaskBase> tasks_;

 public:
  ~ManualLoop();

  ManualLoop(ManualLoop const &) = delete;
  void operator= (ManualLoop const &) = delete;

  ManualLoop(ManualLoop &&) = delete;
  void operator= (ManualLoop &&) = delete;

 public:
  ManualLoop() = default;

  void Submit(task::TaskBase *) noexcept override;

  // Returns the number of completed tasks
  ::std::size_t RunAtMost(::std::size_t limit) noexcept;

  // Returns false if there is no task
  bool RunNext() noexcept {
    return RunAtMost(1) != 0;
  }

  // Returns the number of completed tasks
  ::std::size_t Run() noexcept {
    return RunAtMost(::std::size_t{} - 1);
  }

  [[nodiscard]] bool IsEmpty() const noexcept {
    return tasks_.empty();
  }
};

} // namespace exe::runtime

#endif /* DDVAMP_EXE_RUNTIME_MANUAL_LOOP_HPP_INCLUDED_ */
