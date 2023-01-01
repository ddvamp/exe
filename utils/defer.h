#ifndef DDV_UTILS_DEFER_H_
#define DDV_UTILS_DEFER_H_ 1

#include <concepts>
#include <exception>
#include <type_traits>
#include <utility>

namespace utils {

namespace detail {

template <typename T>
concept suitable_for_deferred_action =
	::std::is_class_v<T> &&
	::std::is_move_constructible_v<T> &&
	::std::is_destructible_v<T> &&
	::std::is_invocable_v<T &> &&
	::std::is_void_v<::std::invoke_result_t<T &>>;

template <typename Policy>
concept guard_policy = requires (Policy const &p)
{
	{ p.should_be_activated() } noexcept -> ::std::same_as<bool>;
	requires ::std::is_default_constructible_v<Policy>;
	requires ::std::is_destructible_v<Policy>;
};

struct default_guard_policy {
	bool should_be_activated() const noexcept
	{
		return true;
	}
};

struct success_guard_policy {
	int uncaught_on_creating_ = ::std::uncaught_exceptions();

	bool should_be_activated() const noexcept
	{
		return ::std::uncaught_exceptions() <= uncaught_on_creating_;
	}
};

struct failure_guard_policy {
	int uncaught_on_creating_ = ::std::uncaught_exceptions();

	bool should_be_activated() const noexcept
	{
		return ::std::uncaught_exceptions() > uncaught_on_creating_;
	}
};

} // namespace detail

// RAII class for performing an action at the end of a scope
template <detail::suitable_for_deferred_action T>
class [[nodiscard]] defer {
protected:
	[[no_unique_address]] T action_;

public:
	~defer()
		noexcept (
			::std::is_nothrow_invocable_v<T &> &&
			::std::is_nothrow_destructible_v<T>
		)
	{
		action_();
	}

	defer(defer const &) = delete;
	void operator= (defer const &) = delete;

	defer(defer &&) = delete;
	void operator= (defer &&) = delete;

public:
	explicit defer(T const &act)
		noexcept (::std::is_nothrow_copy_constructible_v<T>)
		requires (::std::is_copy_constructible_v<T>)
		: action_(act)
	{}

	explicit defer(T &&act)
		noexcept (::std::is_nothrow_move_constructible_v<T>)
		: action_(::std::move(act))
	{}
};

// RAII class for performing an action at the end of a scope
// if it is active and the conditions are met
template <
	detail::suitable_for_deferred_action T,
	detail::guard_policy Policy = detail::default_guard_policy
>
class [[nodiscard]] scope_guard : protected Policy {
protected:
	bool active_;
	[[no_unique_address]] T action_;

public:
	~scope_guard()
		noexcept (
			::std::is_nothrow_invocable_v<T &> &&
			::std::is_nothrow_destructible_v<T> &&
			::std::is_nothrow_destructible_v<Policy>
		)
	{
		if (should_be_activated()) {
			action_();
		}
	}

	scope_guard(scope_guard const &) = delete;
	void operator= (scope_guard const &) = delete;

	scope_guard(scope_guard &&that) noexcept
		requires (
			::std::is_nothrow_move_constructible_v<T> &&
			::std::is_nothrow_move_constructible_v<Policy>
		)
		: Policy(::std::move(that))
		, action_(::std::move(that.action_))
		, active_(::std::exchange(that.active_, false))
	{}
	void operator= (scope_guard &&) = delete;

public:
	explicit scope_guard(T const &act)
		noexcept (
			::std::is_nothrow_default_constructible_v<Policy> &&
			::std::is_nothrow_copy_constructible_v<T>
		)
		requires (::std::is_copy_constructible_v<T>)
		: Policy()
		, action_(act)
		, active_(true)
	{}

	explicit scope_guard(T &&act)
		noexcept (
			::std::is_nothrow_default_constructible_v<Policy> &&
			::std::is_nothrow_move_constructible_v<T>
		)
		: Policy()
		, action_(::std::move(act))
		, active_(true)
	{}

public:
	bool should_be_activated() const noexcept
	{
		return active_ && Policy::should_be_activated();
	}

	void enable() noexcept
	{
		active_ = true;
	}

	void disable() noexcept
	{
		active_ = false;
	}

	void activate_under_policy() &&
		noexcept (::std::is_nothrow_invocable_v<T &>)
	{
		if (Policy::should_be_activated()) {
			active_ = false;
			action_();
		}
	}

	void activate_if_should_be() &&
		noexcept (::std::is_nothrow_invocable_v<T &>)
	{
		if (should_be_activated()) {
			active_ = false;
			action_();
		}
	}
};

// CTAD for alias templates (P1814R0)
#if (defined(__GNUC__) || defined(__GNUG__)) && \
	!defined(__clang__) && !defined(__INTEL_COMPILER)

template <typename Policy>
struct with_policy {
	template <typename T>
	using scope_guard = utils::scope_guard<T, Policy>;
};

template <typename T>
using scope_success = scope_guard<T, detail::defer::success_guard_policy>;

template <typename T>
using scope_failure = scope_guard<T, detail::defer::failure_guard_policy>;

#else

template <typename Policy>
struct with_policy {
	template <typename T>
	class [[nodiscard]] scope_guard : public utils::scope_guard<T, Policy> {
		using utils::scope_guard<T, Policy>::scope_guard;
	};

	template <typename T>
	scope_guard(T) -> scope_guard<T>;
};

template <typename T>
class [[nodiscard]] scope_success :
	public scope_guard<T, detail::success_guard_policy> {
	using scope_guard<T, detail::success_guard_policy>::scope_guard;
};

template <typename T>
scope_success(T) -> scope_success<T>;

template <typename T>
class [[nodiscard]] scope_failure :
	public scope_guard<T, detail::failure_guard_policy> {
	using scope_guard<T, detail::failure_guard_policy>::scope_guard;
};

template <typename T>
scope_failure(T) -> scope_failure<T>;

#endif

} // namespace utils

#endif /* DDV_UTILS_DEFER_H_ */
