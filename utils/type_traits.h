#ifndef DDV_UTILS_TYPE_TRAITS_H_
#define DDV_UTILS_TYPE_TRAITS_H_ 1

#include <functional>
#include <type_traits>
#include <utility>

namespace utils {

// true if and only if T is a specialization of Template
template <typename T, template <typename ...> typename Template>
inline constexpr bool is_specialization_v = false;

template <template <typename ...> typename Template, typename ...Ts>
inline constexpr bool is_specialization_v<Template<Ts...>, Template> = true;

template <typename T, template <typename ...> typename Template>
struct is_specialization
	: ::std::bool_constant<is_specialization_v<T, Template>>
{};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename ...Ts>
inline constexpr bool is_any_of_v = (::std::is_same_v<T, Ts> || ...);

template <typename T, typename ...Ts>
struct is_any_of : ::std::bool_constant<is_any_of_v<T, Ts...>> {};

////////////////////////////////////////////////////////////////////////////////

// determine whether T can be direct-initialized with
// result of applying INVOKE operation to F and Args...
template <typename T, typename Fn, typename ...Args>
inline constexpr bool is_constructible_with_v = requires {
	T(::std::invoke(::std::declval<F>(), ::std::declval<Args>()...));
};

template <typename T, typename Fn, typename ...Args>
struct is_constructible_with
	: ::std::bool_constant<is_constructible_with_v<T, Fn, Args...>>
{};

////////////////////////////////////////////////////////////////////////////////

// determine whether direct-initialization of T from
// result of applying INVOKE operation to F and Args...
// is both valid and not potentially-throwing
template <typename T, typename Fn, typename ...Args>
inline constexpr bool is_nothrow_constructible_with_v = requires {
	{T(::std::invoke(::std::declval<F>(), ::std::declval<Args>()...))} noexcept;
};

template <typename T, typename Fn, typename ...Args>
struct is_nothrow_constructible_with
	: ::std::bool_constant<is_nothrow_constructible_with_v<T, Fn, Args...>>
{};

} // namespace utils

#endif /* DDV_UTILS_TYPE_TRAITS_H_ */
