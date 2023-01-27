#ifndef DDV_COROUTINE_COROUTINE_H_
#define DDV_COROUTINE_COROUTINE_H_ 1

#include <cstddef>
#include <utility>

#include "context/stack.h"

#include "exe/coroutine/impl.h"

namespace exe::coroutine {

// Stackful subroutine for non-preemptive multitasking,
// that allow execution to be suspended and resumed
class [[nodiscard]] Coroutine {
private:
	::context::Stack stack_;
	CoroutineImpl impl_;

public:
	// precondition: routine == true
	explicit Coroutine(Routine &&routine);

	[[nodiscard]] bool isActive() const noexcept
	{
		return impl_.isActive();
	}

	[[nodiscard]] bool isCompleted() const noexcept
	{
		return impl_.isCompleted();
	}

	// Entry point
	// 
	// Precondition: isCompleted() == false && isActive() == false
	void resume();

	// Exit point
	//
	// Precondition: coroutine context
	static void suspend() noexcept;

private:
	static ::context::Stack allocateStack()
	{
		constexpr ::std::size_t kStackPages = 16;
		return ::context::Stack::allocatePages(kStackPages);
	}
};

/* API */

// Exit point
//
// Precondition: coroutine context
void suspend() noexcept;

} // namespace exe::coroutine

#endif /* DDV_COROUTINE_COROUTINE_H_ */
