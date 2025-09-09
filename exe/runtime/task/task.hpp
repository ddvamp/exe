//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_EXECUTORS_TASK_HPP_INCLUDED_
#define DDVAMP_EXE_EXECUTORS_TASK_HPP_INCLUDED_ 1

#include "concurrency/intrusive/forward_list.hpp"

namespace exe::runtime {

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

} // namespace exe::runtime

#endif /* DDVAMP_EXE_EXECUTORS_TASK_HPP_INCLUDED_ */
