// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <utility>

#include "exe/coroutine/coroutine.h"

#include "util/debug.h"
#include "util/defer.h"

namespace exe::coroutine {

namespace {

thread_local Coroutine *current = nullptr;

// debug checking of ctor precondition
Routine &check(Routine &routine) noexcept
{
	UTIL_ASSERT(routine, "empty routine on coroutine creation");

	return routine;
}

} // namespace


////////////////////////////////////////////////////////////////////////////////


Coroutine::Coroutine(Routine &&routine)
	: stack_(allocateStack())
	, impl_(::std::move(check(routine)), stack_.view())
{}

void Coroutine::resume()
{
	UTIL_ASSERT(!isCompleted(), "resuming a completed coroutine");
	UTIL_ASSERT(!isActive(), "coroutine is already active");

	auto cleanup = ::util::rollback_exchange(current, this);

	impl_.resume();
}

/* static */ void Coroutine::suspend() noexcept
{
	UTIL_ASSERT(current, "not in the coroutine context");

	current->impl_.suspend();
}

void suspend() noexcept
{
	Coroutine::suspend();
}

} // namespace exe::coroutine
