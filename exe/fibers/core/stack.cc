#include <mutex>
#include <stack>
#include <utility>

#include "exe/fibers/core/stack.h"

#include "utils/abort.h"
#include "utils/debug.h"
#include "utils/defer.h"

namespace exe::fibers {

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
				auto cleanup = ::utils::defer{
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
		UTILS_ASSERT(stack.allocationSize() != 0, "stack in moved-from state");

		::std::lock_guard lock{m_};

		pool_.push(::std::move(stack));
	} catch (...) {
		UTILS_ABORT("too bad");
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

} // namespace exe::fibers
