#ifndef DDV_EXE_FUTURES_CORE_PROMISE_H_
#define DDV_EXE_FUTURES_CORE_PROMISE_H_ 1

#include <type_traits>

#include "exe/futures/core/detail/shared_state.h"

#include "result/result.h"

#include "utils/utility.h"

namespace exe::futures {

template <typename>
struct Contract;

template <typename T>
class Promise : protected detail::HoldState<T> {
	template <::utils::suitable_for_result U>
	friend Contract<U> makeContractVia(executors::IExecutor &);

	using Base = detail::HoldState<T>;

	using Base::getState;
	using Base::release;
	using Base::reset;

	using Result = Base::Result;

public:
	bool setResult(Result const &result) &&
		noexcept (::std::is_nothrow_copy_constructible_v<Result>)
	{
		auto retval = getState().setResult(result);
		reset();
		return retval;
	}

	bool setResult(Result &&result) &&
		noexcept (::std::is_nothrow_move_constructible_v<Result>)
	{
		auto retval = getState().setResult(::std::move(result));
		reset();
		return retval;
	}

	bool setError(::utils::error error) && noexcept
	{
		return release()->setResult(::std::move(error));
	}

private:
	explicit Promise(detail::StateRef<T> &&state) noexcept
		: Base(::std::move(state))
	{}
};

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_CORE_PROMISE_H_ */
