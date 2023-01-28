// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

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
