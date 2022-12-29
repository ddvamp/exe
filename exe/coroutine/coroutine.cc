#include <utility>

#include "exe/coroutine/coroutine.h"

#include "utils/assert.h"
#include "utils/defer.h"

namespace exe::coroutine {

namespace {

thread_local Coroutine *current = nullptr;

} // namespace

void Coroutine::resume()
{
	auto rollback = ::utils::defer{
		[prev = ::std::exchange(current, this)]() noexcept {
			current = prev;
		}
	};

	impl_.resume();
}

void Coroutine::suspend() noexcept
{
	current->impl_.suspend();
}

void suspend() noexcept
{
	Coroutine::suspend();
}

} // namespace exe::coroutine
