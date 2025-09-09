//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <mutex>
#include <stack>
#include <utility>

#include "exe/fiber/core/stack.hpp"

#include "util/abort.hpp"
#include "util/debug.hpp"
#include "util/defer.hpp"

namespace exe::fiber {

namespace {

class StackAllocator {
private:
	::std::stack<Stack> pool_;
	::std::mutex m_; // protects the pool_

public:
	Stack allocate()
	{
		{
			::std::lock_guard lock{m_};

			if (!pool_.empty()) [[likely]] {
				auto cleanup = ::util::defer{
					[&p = pool_]() noexcept {
						p.pop();
					}
				};

				return ::std::move(pool_.top());
			}
		}

		return allocateNewStack();
	}

	// TODO: harmful noexcept
	void deallocate(Stack &stack) noexcept try
	{
		UTIL_ASSERT(stack.allocationSize() != 0, "stack in moved-from state");

		::std::lock_guard lock{m_};

		pool_.push(::std::move(stack));
	} catch (...) {
		UTIL_ABORT("too bad");
	}

private:
	static Stack allocateNewStack()
	{
		constexpr ::std::size_t kStackPages = 16;
		return Stack::allocatePages(kStackPages);
	}
};

StackAllocator allocator;

} // namespace

Stack allocateStack()
{
	return allocator.allocate();
}

void deallocateStack(Stack &&stack) noexcept
{
	allocator.deallocate(stack);
}

} // namespace exe::fiber
