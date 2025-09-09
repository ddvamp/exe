// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_INLINE_H_
#define DDV_EXE_EXECUTORS_INLINE_H_ 1

#include "exe/runtime/task/scheduler.h"

#include "util/debug.h"

namespace exe::runtime {

// Executes task immediately at place
class InlineScheduler : public ISafeScheduler {
public:
	void submit(TaskBase *task) noexcept override
	{
		UTIL_ASSERT(
			task,
			"nullptr instead of the task"
		);

		task->run();
	}
};

[[nodiscard]] inline InlineScheduler &getInlineScheduler() noexcept
{
	static InlineScheduler instance;
	return instance;
}

} // namespace exe::runtime

#endif /* DDV_EXE_EXECUTORS_INLINE_H_ */
