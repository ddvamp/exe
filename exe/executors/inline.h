// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_INLINE_H_
#define DDV_EXE_EXECUTORS_INLINE_H_ 1

#include "exe/executors/executor.h"

#include "utils/debug.h"

namespace exe::executors {

// executes task immediately at place
class InlineExecutor : public IExecutor {
private:
	void doExecute(TaskBase *task) noexcept override
	{
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
