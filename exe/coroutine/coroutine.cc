#include <utility>

#include "exe/coroutine/coroutine.h"

#include "utils/debug.h"
#include "utils/defer.h"

namespace exe::coroutine {

namespace {

thread_local Coroutine *current = nullptr;

// debug checking of ctor precondition
Routine &check(Routine &routine) noexcept
{
	UTILS_ASSERT(routine, "empty routine on coroutine creation");

	return routine;
}

} // namespace


////////////////////////////////////////////////////////////////////////////////


explicit Coroutine::Coroutine(Routine &&routine)
	: stack_(allocateStack())
	, impl_(::std::move(check(routine)), stack_.view())
{}

void Coroutine::resume()
{
	UTILS_ASSERT(!isCompleted(), "resuming a completed coroutine");
	UTILS_ASSERT(!isActive(), "coroutine is already active");

	auto cleanup = ::utils::rollback_exchange(current, this);

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
