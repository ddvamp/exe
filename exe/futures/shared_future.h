// 3. combinator (?terminator) futures::Share(IExecutor)
// 6. shared_future copyable and one-shot

// 1. отложенна€ установка executor в FutureHolder
// 2. SemiFuture concept
// 3. rename utils::builder -> utils::lazy_construct (?)
// 4. passing futures to mutator functions by rvalue reference instead of by value
// 5. ???protected (?private) Future (as concept) constructor from SemiFuture (as concept)
// 6. заменить optional на union
// 7. ѕодумать над изменением intrusive_node и TaskBase (выделить TaskNode)

#include <atomic>
#include <functional>
#include <utility>

#include "concurrency/one_shot_event.h"

#include "exe/executors/task.h"
#include "exe/executors/executor.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

#include "utils/debug.h"
#include "utils/refer/ref_counted_ptr.h"

namespace exe::futures {

namespace detail {

template <::utils::suitable_for_result T>
	requires (traits::is_nothrow_move_constructible_v<T>)
class SharedFutureState;

} // namespace detail

template <typename T>
class SharedFutureResultPtr {
	friend detail::SharedFutureState<T>;

public:
	using Result = detail::SharedFutureState<T>::Result;

private:
	::utils::RefCountedPtr<detail::SharedFutureState<T>> state_;

public:
	[[nodiscard]] Result const &get() const noexcept
	{
		return state_->get();
	}

private:
	explicit SharedFutureResultPtr(
		detail::SharedFutureState<T> *state) noexcept
		: state_(state)
	{}
};

template <typename T>
using Callback =
	::std::move_only_function<void(SharedFutureResultPtr<T>) noexcept>;

namespace detail {

template <typename T>
class SharedFutureResult {
protected:
	using Result = ::utils::result<T>;

private:
	union { Result result_; };
	::concurrency::OneShotEvent is_ready_;

protected:
	~SharedFutureResult() noexcept
	{
		result_.~Result();
	}

	SharedFutureResult(SharedFutureResult const &) = delete;
	void operator= (SharedFutureResult const &) = delete;

	SharedFutureResult(SharedFutureResult &&) = delete;
	void operator= (SharedFutureResult &&) = delete;

protected:
	SharedFutureResult() noexcept {}

	[[nodiscard]] bool isReady() const noexcept
	{
		return is_ready_.isReady();
	}

	void wait() noexcept
	{
		is_ready_.wait();
	}

	[[nodiscard]] Result const &get() const noexcept
	{
		return result_;
	}

	void set(Result &&result) noexcept
	{
		::new (&result_) Result(::std::move(result));
		is_ready_.notify();
	}
};

struct SharedFutureStateData {
	struct DummyTask : executors::TaskBase {
		void run() noexcept override
		{
			// do nothing
		}
	};

	executors::INothrowExecutor &where_;
	DummyTask dummy_;
	executors::TaskBase *head_ = &dummy_;
	::std::atomic<executors::TaskBase *> tail_ = &dummy_;
};

template <::utils::suitable_for_result T>
	requires (traits::is_nothrow_move_constructible_v<T>)
class SharedFutureState final
	: private executors::TaskBase
	, private SharedFutureStateData
	, private SharedFutureResult<T>
	, public ::utils::RefCounted<SharedFutureState<T>> {
private:
	using Callback = Callback<T>;
	using Ptr = SharedFutureResultPtr<T>;

public:
	using SharedFutureResult<T>::Result;

public:
	[[nodiscard]] static SharedFutureState *create(
		executors::INothrowExecutor &where)
	{
		return ::new SharedFutureState(where);
	}
	
	void destroySelf() const noexcept
	{
		delete this;
	}

public:
	using SharedFutureResult<T>::get;

	[[nodiscard]] Ptr getResult() noexcept
	{
		this->wait();
		return Ptr(this);
	}

	void setResult(Result &&result) noexcept
	{
		this->set(::std::move(result));

		if (TaskBase *expected = nullptr;
			!dummy_.next_.compare_exchange_strong(
				expected,
				&dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			)
		) {
			submitSelf();
		}

		this->decRef();
	}

	void addCallback(executors::INothrowExecutor &where, Callback &&what)
	{
		auto const task = makeCallbackTask(where, ::std::move(what));

		if (this->isReady()) {
			scheduleTask(task);
			return;
		}

		auto const prev =
			tail_.exchange(task, ::std::memory_order_acq_rel)->
			next_.exchange(task, ::std::memory_order_acq_rel);

		if (prev) [[unlikely]] {
			submitSelf();
		}
	}

	void detach() const noexcept
	{
		this->decRef();
	}

private:
	explicit SharedFutureState(executors::INothrowExecutor &where) noexcept
		: SharedFutureStateData(where)
		, ::utils::RefCounted<SharedFutureState<T>>(2)
	{}

	void submitSelf() noexcept
	{
		where_.submit(this);
	}

	void run() noexcept override
	{
		auto curr = head_;
		auto next = curr->next_.load(::std::memory_order_acquire);

		if (curr == &dummy_) [[likely]] {
			dummy_.link(&dummy_);
			goto get_next;
		}

	schedule:
		scheduleTask(curr);

	get_next:
		curr = next;

		if ((next = curr->next_.load(::std::memory_order_acquire))) {
			goto schedule;
		}

		if (auto expected = curr;
			tail_.compare_exchange_strong(
				expected,
				head_ = &dummy_,
				::std::memory_order_release,
				::std::memory_order_relaxed
			)
		) {
			scheduleTask(curr);
			return;
		}

		if (
			!curr->next_.compare_exchange_strong(
				next,
				head_ = curr,
				::std::memory_order_release,
				::std::memory_order_acquire
			)
		) {
			goto schedule;
		}
	}

private:
	class Task : public executors::TaskBase {
	private:
		executors::INothrowExecutor &where_;
		Callback callback_;
		Ptr result_;

	public:
		Task(executors::INothrowExecutor &where,
			Callback &&cb, Ptr &&res) noexcept
			: where_(where)
			, callback_(::std::move(cb))
			, result_(::std::move(res))
		{}

		void schedule() noexcept
		{
			where_.submit(this);
		}

	private:
		void run() noexcept override
		{
			callback_(::std::move(result_));
			destroySelf();
		}

		void destroySelf() const noexcept
		{
			delete this;
		}
	};

	auto *makeCallbackTask(executors::INothrowExecutor &where, Callback &&what)
	{
		return ::new Task(where, ::std::move(what), Ptr(this));
	}

	static void scheduleTask(executors::TaskBase *const task) noexcept
	{
		static_cast<Task *>(task)->schedule();
	}
};

} // namespace detail

} // namespace exe::futures
