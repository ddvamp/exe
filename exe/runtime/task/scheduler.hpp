// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_EXECUTORS_EXECUTOR_HPP_INCLUDED_
#define DDVAMP_EXE_EXECUTORS_EXECUTOR_HPP_INCLUDED_ 1

#include <concepts>

#include "exe/runtime/task/task.hpp"

namespace exe::runtime {

// Schedulers are to function execution as allocators are to memory allocation
class IScheduler {
public:
	virtual ~IScheduler() = default;

	virtual void submit(TaskBase *) = 0;
};

class ISafeScheduler : public IScheduler {
public:
	void submit(TaskBase *) noexcept override = 0;
};

////////////////////////////////////////////////////////////////////////////////

namespace concepts {

template <typename E>
concept Scheduler = ::std::derived_from<E, IScheduler>;

template <typename E>
concept SafeScheduler = ::std::derived_from<E, ISafeScheduler>;

} // namespace concepts

} // namespace exe::runtime

#endif /* DDVAMP_EXE_EXECUTORS_EXECUTOR_HPP_INCLUDED_ */
