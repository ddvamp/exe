#include "run_loop.hpp"

#include <utils/debug/assert.hpp>
#include <utils/utility.hpp>

namespace exe::sched {

RunLoop::~RunLoop() {
  UTILS_ASSERT(IsEmpty(), "RunLoop contains tasks when destroyed");
}

/* virtual */ void RunLoop::Submit(task::TaskBase *task) noexcept {
  UTILS_ASSERT(task, "nullptr instead of the task");
  task->link(nullptr);
  if (IsEmpty()) {
    head_ = tail_ = task;
  } else {
    tail_->link(task);
    tail_ = task;
  }
}

::std::size_t RunLoop::RunAtMost(::std::size_t const limit) noexcept {
  auto remains = limit;
  while (::utils::all_of(!IsEmpty(), remains != 0)) {
    PopNext()->Run();
    --remains;
  }
  return limit - remains;
}

task::TaskBase *RunLoop::PopNext() noexcept {
  return ::std::exchange(head_, head_->next_.load(::std::memory_order_relaxed));
}

}  // namespace exe::sched
