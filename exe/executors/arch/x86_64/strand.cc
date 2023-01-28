// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "exe/executors/arch/x86_64/strand.h"

#include "utils/abort.h"
#include "utils/debug.h"
#include "utils/utility.h"

namespace exe::executors {

/* virtual */ void Strand::execute(TaskBase *task) noexcept
{
	UTILS_ASSERT(task, "nullptr instead of a task");
	
	task->next_.store(nullptr, ::std::memory_order_relaxed);

	auto prev = tail_.exchange(task);

	auto idle = static_cast<bool>(prev->next_.exchange(task));
	if (idle) {
		// start a new critical section
		// (relying on correct synchronization by executor)
		head_ = prev;

		// TODO: exception handling
		try {
			executor_.execute(this);
		} catch (...) {
			UTILS_ABORT(
				"exception while trying to start "
				"a critical section of strand"
			);
		}
	}
}

/* virtual */ void Strand::run() noexcept
{
	auto curr = head_;
	auto next = curr->next_.load(::std::memory_order_relaxed);

	do {
		// at this point curr is the only pointer to its object
		// (after run, curr object doesn't exist)
		curr->run();
		curr = next;
	} while (
		// be optimistic: in case of a high load,
		// we will immediately get the following task
		(next = curr->next_.load(::std::memory_order_acquire)) ||

		(
			// are there any other tasks?
			tail_.compare_exchange_strong(
				::utils::temporary(::utils::decay_copy(curr)),
				&dummy_
			) &&
			// cmpxchg -> true: there are no other tasks

			// at this point curr is the only pointer to its object
			// (after run, curr object doesn't exist)
			(curr->run(), curr = &dummy_),

			// if the task has not been linked yet, we give control
			(next = curr->next_.load(::std::memory_order_acquire)) ||
			(next = curr->next_.exchange(curr))
		)
	);
}

} // namespace exe::executors
