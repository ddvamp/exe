namespace utils {

template <typename T>
struct unwrap_result {
	using type = T;
};

template <typename T>
requires (is_result_v<T>)
struct unwrap_result<T> {
	using type = typename T::value_type;
};

template <typename T>
using unwrap_result_t = typename unwrap_result<T>::type;


////////////////////////////////////////////////////////////////////////////////


template <typename T>
struct to_result {
	using type = result<T>;
};

template <typename T>
requires (is_result_v<T>)
struct to_result<T> {
	using type = T;
};

template <typename T>
using to_result_t = typename to_result<T>::type;


////////////////////////////////////////////////////////////////////////////////


namespace detail {

template <typename ...>
struct invoke_traits {
	using is_invocable = ::std::false_type;
	using is_nothrow_invocable = ::std::false_type;
};

template <typename F, typename ...Args>
requires requires {
	typename result<::std::remove_cvref_t<::std::invoke_result_t<F, Args...>>>;	
}
struct invoke_traits<F, Args...> {
	using type = result<::std::remove_cvref_t<::std::invoke_result_t<F, Args...>>>;
	using is_invocable = ::std::true_type;
	using is_nothrow_invocable = ::std::is_nothrow_invocable<F, Args...>;
};

} // namespace detail

template <typename F, typename ...Args>
struct invoke_result : detail::invoke_traits<F, Args...> {};

template <typename F, typename ...Args>
using invoke_result_t = typename detail::invoke_traits<F, Args...>::type;

template <typename F, typename ...Args>
struct is_invocable : detail::invoke_traits<F, Args...>::is_invocable {};

template <typename F, typename ...Args>
inline constexpr bool is_invocable_v =
	detail::invoke_traits<F, Args...>::is_invocable::value;

template <typename F, typename ...Args>
struct is_nothrow_invocable
	: detail::invoke_traits<F, Args...>::is_nothrow_invocable {};

template <typename F, typename ...Args>
inline constexpr bool is_nothrow_invocable_v =
	detail::invoke_traits<F, Args...>::is_nothrow_invocable::value;


////////////////////////////////////////////////////////////////////////////////


namespace detail {

struct invoker_result {
	template <typename F, typename ...Args>
	inline static auto call(F &&f, Args &&...args) noexcept
	{	
		using result = invoke_result_t<F, Args...>;

		if constexpr (is_nothrow_invocable_v<F, Args...>) {
			return result(
				invoke_place,
				::std::forward<F>(f),
				::std::forward<Args>(args)...
			);
		} else try {
			return result(
				invoke_place,
				::std::forward<F>(f),
				::std::forward<Args>(args)...
			);
		} catch (...) {
			return result(error_place, ::std::current_exception());
		}
	}
};

struct invoker_status {
	template <typename F, typename ...Args>
	inline static auto call(F &&f, Args &&...args) noexcept
	{	
		if constexpr (is_nothrow_invocable_v<F, Args...>) {
			::std::invoke(::std::forward<F>(f), ::std::forward<Args>(args)...);
			return status(value_place);
		} else try {
			::std::invoke(::std::forward<F>(f), ::std::forward<Args>(args)...);
			return status(value_place);
		} catch (...) {
			return status(error_place, ::std::current_exception());
		}
	}
};

template <typename T>
struct invoker : invoker_result {};

template <>
struct invoker<status> : invoker_status {};

} // namespace detail


////////////////////////////////////////////////////////////////////////////////


template <typename F, typename ...Args>
inline invoke_result_t<F, Args...> invoke(F &&f, Args &&...args) noexcept
	requires (is_invocable_v<F, Args...>)
{
	return detail::invoker<invoke_result_t<F, Args...>>::call(
		::std::forward<F>(f),
		::std::forward<Args>(args)...
	);
}

} // namespace utils
