#include "exe/executors/inline.h"

#include "utils/debug.h"

namespace exe::executors {

/* virtual */ void InlineExecutor::execute(TaskBase *task) noexcept
{
	UTILS_ASSERT(task, "inline executor got nullptr instead of a task");
	task->run();
}

} // namespace exe::executors
