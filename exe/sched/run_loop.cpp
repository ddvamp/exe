#include "run_loop.hpp"

#include <util/debug/assert.hpp>
#include <util/utility.hpp>

namespace exe::sched {

RunLoop::~RunLoop() {
  UTIL_ASSERT(IsEmpty(), "RunLoop contains tasks when destroyed");
}

/* virtual */ void RunLoop::Submit(task::TaskBase *task) noexcept {
  UTIL_ASSERT(task, "nullptr instead of the task");
  task->Link(nullptr);
  if (IsEmpty()) {
    head_ = tail_ = task;
  } else {
    tail_->Link(task);
    tail_ = task;
  }
}

::std::size_t RunLoop::RunAtMost(::std::size_t const limit) noexcept {
  auto remains = limit;
  while (::util::all_of(!IsEmpty(), remains != 0)) {
    Pop()->Run();
    --remains;
  }
  return limit - remains;
}

task::TaskBase *RunLoop::Pop() noexcept {
  return ::std::exchange(head_, head_->next_.load(::std::memory_order_relaxed));
}

}  // namespace exe::sched
