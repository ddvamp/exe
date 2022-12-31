#include "exe/executors/arch/x86_64/strand.h"

#include "utils/abort.h"
#include "utils/debug.h"

namespace exe::executors {

// wait-free task submission
// 
// precondition: task && task->next_ == nullptr
/* virtual */ void Strand::execute(TaskBase *task) noexcept
{
	UTILS_ASSERT(
		task,
		"violation of the execute precondition"
	);
	UTILS_ASSERT(
		!task->next_.load(::std::memory_order_relaxed),
		"violation of the execute precondition"
	);

	auto *prev = tail_.exchange(task);

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

} // namespace exe::executors
