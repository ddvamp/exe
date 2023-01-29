// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_EXECUTOR_H_
#define DDV_EXE_EXECUTORS_EXECUTOR_H_ 1

#include <source_location>

#include "exe/executors/task.h"

#include "utils/debug.h"

namespace exe::executors {

// executors are to function execution as allocators are to memory allocation
class IExecutor {
public:
	virtual ~IExecutor() = default;

	// precondition: task != nullptr
	void execute(TaskBase *task, 
		[[maybe_unused]] ::std::source_location location =
		::std::source_location::current())
	{
		UTILS_ASSERT(task, "nullptr instead of a task", location);
		doExecute(task);
	}

private:
	virtual void doExecute(TaskBase *) = 0;
};

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_EXECUTOR_H_ */
