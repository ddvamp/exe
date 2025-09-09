// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_TASK_H_
#define DDV_EXE_EXECUTORS_TASK_H_ 1

#include "concurrency/intrusive/forward_list.h"

namespace exe::executors {

class ITask {
public:
	virtual ~ITask() noexcept = default;

	// the user must take care of passing arguments, returning values,
	// and handling exceptions himself
	virtual void run() noexcept = 0;
};

class TaskBase
	: public ITask
	, public ::concurrency::intrusive_concurrent_forward_list_node<TaskBase> {};

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_TASK_H_ */
