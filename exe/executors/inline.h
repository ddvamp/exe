// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_INLINE_H_
#define DDV_EXE_EXECUTORS_INLINE_H_ 1

#include "exe/executors/executor.h"

#include "util/debug.h"

namespace exe::executors {

// Executes task immediately at place
class InlineExecutor : public INothrowExecutor {
public:
	void submit(TaskBase *task) noexcept override
	{
		UTILS_ASSERT(
			task,
			"nullptr instead of the task"
		);

		task->run();
	}
};

[[nodiscard]] inline InlineExecutor &getInlineExecutor() noexcept
{
	static InlineExecutor instance;
	return instance;
}

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_INLINE_H_ */
