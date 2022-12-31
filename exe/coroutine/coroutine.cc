#include <utility>

#include "exe/coroutine/coroutine.h"

#include "utils/debug.h"
#include "utils/defer.h"

namespace exe::coroutine {

namespace {

thread_local Coroutine *current = nullptr;

} // namespace

void Coroutine::resume()
{
	UTILS_ASSERT(!is_active_, "resume, but the coroutine is already active");

#ifndef UTILS_DISABLE_ASSERT
	auto _ = ::utils::defer{
		[&flag = is_active_ = true]() noexcept {
			flag = false;
		}
	};
#endif

	auto rollback = ::utils::defer{
		[prev = ::std::exchange(current, this)]() noexcept {
			current = prev;
		}
	};

	impl_.resume();
}

/* static */ void Coroutine::suspend() noexcept
{
	UTILS_ASSERT(current, "suspend, but not in the coroutine context");

	current->impl_.suspend();
}

void suspend() noexcept
{
	Coroutine::suspend();
}

} // namespace exe::coroutine
