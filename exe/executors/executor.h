// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_EXECUTOR_H_
#define DDV_EXE_EXECUTORS_EXECUTOR_H_ 1

#include <concepts>

#include "exe/executors/task.h"

namespace exe::executors {

// Executors are to function execution as allocators are to memory allocation
class IExecutor {
public:
	virtual ~IExecutor() = default;

	virtual void submit(TaskBase *) = 0;
};

class INothrowExecutor : public IExecutor {
public:
	void submit(TaskBase *) noexcept override = 0;
};

////////////////////////////////////////////////////////////////////////////////

namespace concepts {

template <typename E>
concept Executor = ::std::derived_from<E, IExecutor>;

template <typename E>
concept NothrowExecutor = ::std::derived_from<E, INothrowExecutor>;

} // namespace concepts

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_EXECUTOR_H_ */
