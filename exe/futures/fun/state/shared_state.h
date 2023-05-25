// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_STATE_SHARED_STATE_H_
#define DDV_EXE_FUTURES_FUN_STATE_SHARED_STATE_H_ 1

#include <optional>
#include <type_traits>
#include <utility>

#include "concurrency/meeting.h"

#include "exe/executors/executor.h"
#include "exe/executors/task.h"
#include "exe/futures/fun/state/callback.h"

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
class SharedState : public executors::TaskBase {
public:
	using Result = ::utils::result<T>;
	using Callback = futures::Callback<T>;

private:
	::std::optional<Result> result_;
	Callback callback_;
	::concurrency::Meeting meeting_{2};
	executors::IExecutor *executor_ = nullptr;

public:
	[[nodiscard]] static SharedState *create()
	{
		return ::new SharedState();
	}

	static void destroy(SharedState *state) noexcept
	{
		state->destroySelf();
	}

	[[nodiscard]] executors::IExecutor &getExecutor() const noexcept
	{
		return *executor_;
	}

	void setExecutor(executors::IExecutor &where) noexcept
	{
		executor_ = &where;
	}

	void setResult(Result &&result)
		noexcept (::std::is_nothrow_move_constructible_v<Result>)
	{
		result_.emplace(::std::move(result));
		notify();
	}

	void setCallback(Callback &&callback) noexcept
	{
		callback_ = ::std::move(callback);
		notify();
	}

private:
	SharedState() noexcept = default;

	void run() noexcept override
	{
		callback_(*::std::move(result_));
		destroySelf();
	}

	void scheduleCallback() noexcept
	{
		// TODO: exception handling
		try {
			executor_->execute(this);
		} catch (...) {
			UTILS_ABORT("exception when scheduling execution of callback");
		}
	}

	void notify() noexcept
	{
		if (meeting_.takesPlace()) {
			scheduleCallback();
		}
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

template <typename T>
class HoldState {
protected:
	using value_type = T;

	using State = SharedState<value_type>;
	using Result = State::Result;
	using Callback = State::Callback;

private:
	State *state_;

protected:
	~HoldState()
	{
		UTILS_ASSERT(!hasState(), "destruction of unused HoldState");
	}

	HoldState(HoldState const &) = delete;
	void operator= (HoldState const &) = delete;

	HoldState(HoldState &&that) noexcept
		: HoldState(that.release())
	{}
	void operator= (HoldState &&) = delete;

protected:
	explicit HoldState(State *state) noexcept
		: state_(state)
	{}

	[[nodiscard]] bool hasState() const noexcept
	{
		return state_;
	}

	[[nodiscard]] State &getState() noexcept
	{
		UTILS_RUN(checkState);
		return *state_;
	}

	[[nodiscard]] State const &getState() const noexcept
	{
		UTILS_RUN(checkState);
		return *state_;
	}

	[[nodiscard]] State *release() noexcept
	{
		UTILS_RUN(checkState);
		return ::std::exchange(state_, nullptr);
	}

	void reset() noexcept
	{
		state_ = nullptr;
	}

private:
	[[maybe_unused]] void checkState() const noexcept
	{
		UTILS_CHECK(hasState(), "HoldState does not hold shared state");
	}
};

} // namespace exe::futures::detail

#endif /* DDV_EXE_FUTURES_FUN_STATE_SHARED_STATE_H_ */
