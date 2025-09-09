// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_SAFE_EXECUTOR_H_
#define DDV_EXE_EXECUTORS_SAFE_EXECUTOR_H_ 1

#include <type_traits>

#include "exe/runtime/scheduler.h"

#include "util/abort.h"

namespace exe::runtime {

// A decorator for an scheduler that aborts the program when
// an exception is thrown when submitting a task
template <concepts::Scheduler E>
	requires (!concepts::SafeScheduler<E>)
class SafeScheduler : public ISafeScheduler {
private:
	E &underlying_;

public:
	explicit SafeScheduler(E &underlying) noexcept
		: underlying_(underlying)
	{}

	void submit(TaskBase *task) noexcept override
	{
		try {
			if constexpr (::std::is_abstract_v<E>) {
				underlying_.submit(task);
			} else {
				underlying_.E::submit(task);
			}
		} catch (...) {
			UTIL_ABORT("an exception was thrown when submitting the task");
		}
	}

	[[nodiscard]] E &getUnderlying() const noexcept
	{
		return underlying_;
	}
};

} // namespace exe::runtime

#endif /* DDV_EXE_EXECUTORS_SAFE_EXECUTOR_H_ */
