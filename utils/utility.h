#ifndef DDV_UTILS_UTILITY_H_
#define DDV_UTILS_UTILITY_H_ 1

#include <concepts>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace utils {

/**
 *	@brief		Convert a rvalue to an lvalue.
 *	@param t	A thing of arbitrary type.
 *	@return		The parameter explicitly converted to an lvalue-reference.
 *
 *	This function can be used to convert an prvalue to an lvalue.
 *	In this case temporary materialization occurs.
 */
template <typename T>
[[nodiscard]] inline constexpr T &temporary(T &&t) noexcept
{
	return static_cast<T &>(t);
}

template <typename T>
[[nodiscard]] inline constexpr T &temporary(T &) noexcept = delete;


////////////////////////////////////////////////////////////////////////////////


// Creates decayed copy of an object
// Deprecated since C++23 (auto{} syntax)
template <typename T>
[[nodiscard]] inline constexpr ::std::decay_t<T> decay_copy(T &&t)
	noexcept (::std::is_nothrow_convertible_v<T, ::std::decay_t<T>>)
{
	return ::std::forward<T>(t);
}


////////////////////////////////////////////////////////////////////////////////


// simple utility for combining lambdas
template <typename ...Ts>
struct overloaded : Ts... { using Ts::operator()...; };


////////////////////////////////////////////////////////////////////////////////


// Class that allows to defer the creation of an object
// until it is actually needed, and forward only a reference to its instance
// 
// Attention: the class object lives only until the end of the full expression
template <typename ResultType = void, typename Callable, typename ...Args>
requires (
	::std::is_invocable_r_v<ResultType, Callable, Args...> &&
	!::std::is_void_v<::std::invoke_result_t<Callable, Args...>>
)
[[nodiscard]] inline constexpr auto builder(
	Callable &&fn, Args &&...args) noexcept
{
	using R = ::std::conditional_t<
		::std::is_void_v<ResultType>,
		::std::invoke_result_t<Callable, Args...>,
		ResultType
	>;

	struct builder_t {
		[[nodiscard]] inline constexpr operator R ()
			noexcept (::std::is_nothrow_invocable_r_v<R, Callable, Args...> )
		{
			return ::std::apply(fn_, ::std::move(args_));
		}

		[[no_unique_address]] Callable &&fn_;
		[[no_unique_address]] ::std::tuple<Args &&...> args_;
	};

	return builder_t{
		.fn_ = ::std::forward<Callable>(fn),
		.args_ = ::std::forward_as_tuple(::std::forward<Args>(args)...),
	};
}

} // namespace utils

#endif /* DDV_UTILS_UTILITY_H_ */
