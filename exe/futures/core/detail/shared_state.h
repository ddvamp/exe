#ifndef DDV_EXE_FUTURES_CORE_DETAIL_SHARED_STATE_H_
#define DDV_EXE_FUTURES_CORE_DETAIL_SHARED_STATE_H_ 1

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "concurrency/rendezvous.h"

#include "exe/executors/executor.h"
#include "exe/executors/task.h"
#include "exe/futures/core/callback.h"

#include "result/result.h"

#include "utils/abort.h"
#include "utils/debug.h"

namespace exe::futures::detail {

// Shared state for Future and Promise
// 
// Allows you to pass result from Promise to Future
// and callback in the opposite direction,
// as well as to specify where callback will be called
template <typename T>
class SharedState
	: public executors::TaskBase
	, public ::std::enable_shared_from_this<SharedState> {
public:
	using Result = ::utils::result<T>;
	using Callback = futures::Callback<T>;

private:
	::concurrency::Rendezvous rendezvous_;
	::std::optional<Result> result_;
	Callback callback_;
	executors::IExecutor *executor_;
	::std::shared_ptr<SharedState> self_;

public:
	[[nodiscard]] static ::std::shared_ptr<SharedState> create(
		executors::IExecutor &where)
	{
		return ::std::make_shared<SharedState>(&where);
	}

	void setExecutor(executors::IExecutor &where) noexcept
	{
		executor_ = &where;
	}

	[[nodiscard]] executors::IExecutor &getExecutor() const noexcept
	{
		return *executor_;
	}

	// may be false negative
	[[nodiscard]] bool hasResult() const noexcept
	{
		return rendezvous_.isLatecomer();
	}

	// returns true if execution of callback was scheduled
	[[nodiscard]] bool setResult(Result const &result)
		noexcept (::std::is_nothrow_copy_constructible_v<Result>)
	{
		result_.emplace(result);
		return inform();
	}

	// returns true if execution of callback was scheduled
	[[nodiscard]] bool setResult(Result &&result)
		noexcept (::std::is_nothrow_move_constructible_v<Result>)
	{
		result_.emplace(::std::move(result));
		return inform();
	}

	// returns true if execution of callback was scheduled
	[[nodiscard]] bool setCallback(Callback &&callback) noexcept
	{
		callback_ = ::std::move(callback);
		return inform();
	}

	// Alternative to setCallback
	//
	// Precondition: hasResult() == true
	Result getReadyResult()
		noexcept (::std::is_nothrow_move_constructible_v<Result>)
	{
		UTILS_ASSERT(hasResult(), "shared state has no result");

		return *::std::move(result_);
	}

private:
	explicit SharedState(executors::IExecutor *where) noexcept
		: executor_(where)
	{}

	void run() noexcept override
	{
		callback_(*::std::move(result_));
		self_.reset();
	}

	void scheduleCallback() noexcept
	{
		self_ = this->shared_from_this();

		// TODO: exception handling
		try {
			executor_->execute(this);
		} catch (...) {
			UTILS_ABORT("exception when scheduling execution of callback");
		}
	}

	// if there is both result and callback,
	// schedules execution of callback and returns true
	bool inform() noexcept
	{
		if (rendezvous_.arrive()) {
			scheduleCallback();
			return true;
		}

		return false;
	}
};

template <typename T>
using StateRef = ::std::shared_ptr<SharedState<T>>;

template <typename T>
class HoldState {
private:
	using State = SharedState<T>;

protected:
	using Result = State::Result;
	using Callback = State::Callback;

	StateRef<T> state_;

protected:
	~HoldState()
	{
		UTILS_ASSERT(!hasState(), "destruction of unused HoldState");
	}

	HoldState(HoldState const &) = delete;
	void operator= (HoldState const &) = delete;

	HoldState(HoldState &&) = default;
	HoldState &operator= (HoldState &&) = default;

protected:
	explicit HoldState(StateRef<T> &&state) noexcept
		: state_(::std::move(state))
	{}

	[[nodiscard]] bool hasState() const noexcept
	{
		return state_;
	}

	void reset() noexcept
	{
		state_.reset();
	}

	[[nodiscard]] StateRef<T> release() noexcept
	{
		UTILS_RUN(check);
		return ::std::move(state_);
	}

	[[nodiscard]] State &getState() noexcept
	{
		UTILS_RUN(check);
		return *state_;
	}

	[[nodiscard]] State const &getState() const noexcept
	{
		UTILS_RUN(check);
		return *state_;
	}

private:
	[[maybe_unused]] void check() const noexcept
	{
		UTILS_CHECK(hasState(), "HoldState does not hold shared state");
	}
};

} // namespace exe::futures::detail

#endif /* DDV_EXE_FUTURES_CORE_DETAIL_SHARED_STATE_H_ */
