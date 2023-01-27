#ifndef DDV_EXE_EXECUTORS_INLINE_H_
#define DDV_EXE_EXECUTORS_INLINE_H_ 1

#include "exe/executors/executor.h"

namespace exe::executors {

// executes task immediately at place
class InlineExecutor : public IExecutor {
public:
	// precondition: task != nullptr
	void execute(TaskBase *task) noexcept override;
};

[[nodiscard]] inline IExecutor &getInlineExecutor() noexcept
{
	static InlineExecutor instance;
	return instance;
}

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_INLINE_H_ */
