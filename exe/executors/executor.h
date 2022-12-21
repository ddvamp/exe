#ifndef DDV_EXE_EXECUTORS_EXECUTOR_H_
#define DDV_EXE_EXECUTORS_EXECUTOR_H_ 1

#include "exe/executors/task.h"

namespace exe::executors {

// Executors are to function execution as allocators are to memory allocation
class IExecutor {
public:
	virtual ~IExecutor() = default;

	virtual void execute(TaskBase *task) = 0;
};

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_EXECUTOR_H_ */
