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
#ifndef UTILS_DISABLE_DEBUG
	UTILS_CHECK(!is_active_, "coroutine is already active");

	auto _ = ::utils::rollback_exchange(is_active_, true);
#endif

	auto rollback = ::utils::rollback_exchange(current, this);

	impl_.resume();
}

/* static */ void Coroutine::suspend() noexcept
{
	UTILS_ASSERT(current, "not in the coroutine context");

	current->impl_.suspend();
}

void suspend() noexcept
{
	Coroutine::suspend();
}

} // namespace exe::coroutine
