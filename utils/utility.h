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


// Class that allows to delay the execution of a functor for
// initializing an object until it is actually created
//
// During initialization, type deduction should not take place,
// since the type of the builder itself will be deduced
template <typename Fn>
requires (
	::std::is_class_v<Fn> &&
	::std::is_move_constructible_v<Fn> &&
	::std::is_destructible_v<Fn> &&
	::std::is_invocable_v<Fn &> &&
	!::std::is_void_v<::std::invoke_result_t<Fn &>>
)
class builder {
private:
	[[no_unique_address]] Fn fn_;

public:
	explicit builder(Fn const &fn)
		noexcept (::std::is_nothrow_copy_constructible_v<Fn>)
		requires (::std::is_copy_constructible_v<Fn>)
		: fn_(fn)
	{}

	explicit builder(Fn &&fn)
		noexcept (::std::is_nothrow_move_constructible_v<Fn>)
		: fn_(::std::move(fn))
	{}

	[[nodiscard]] operator decltype(auto) ()
		noexcept (::std::is_nothrow_invocable_v<Fn &>)
	{
		return fn_();
	}
};

} // namespace utils

#endif /* DDV_UTILS_UTILITY_H_ */
