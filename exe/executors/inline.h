#ifndef DDV_EXE_EXECUTORS_INLINE_H_
#define DDV_EXE_EXECUTORS_INLINE_H_ 1

#include "exe/executors/executor.h"

#include "utils/debug.h"

namespace exe::executors {

// executes task immediately at place
class InlineExecutor : public IExecutor {
public:
	// precondition: task != nullptr
	void execute(TaskBase *task) noexcept override
	{
		UTILS_ASSERT(task, "inline executor got nullptr instead of a task");
		task->run();
	}
};

[[nodiscard]] inline IExecutor &getInlineExecutor() noexcept
{
	static InlineExecutor instance;
	return instance;
}

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_INLINE_H_ */
