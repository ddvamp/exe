#ifndef DDV_COROUTINE_COROUTINE_H_
#define DDV_COROUTINE_COROUTINE_H_ 1

#include <cstddef>
#include <utility>

#include "context/stack.h"

#include "exe/coroutine/impl.h"

namespace exe::coroutine {

class Coroutine {
private:
	::context::Stack stack_;
	CoroutineImpl impl_;

#ifndef UTILS_DISABLE_DEBUG
	bool is_active_ = false;
#endif

public:
	explicit Coroutine(Routine routine)
		: stack_(allocateStack())
		, impl_(::std::move(routine), stack_.view())
	{}

	bool isCompleted() const noexcept
	{
		return impl_.isCompleted();
	}

	void resume();

	static void suspend() noexcept;

private:
	static ::context::Stack allocateStack()
	{
		constexpr ::std::size_t kStackPages = 16;
		return ::context::Stack::allocatePages(kStackPages);
	}
};

void suspend() noexcept;

} // namespace exe::coroutine

#endif /* DDV_COROUTINE_COROUTINE_H_ */
