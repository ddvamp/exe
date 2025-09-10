//
// defer.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_DEFER_HPP_INCLUDED_
#define DDVAMP_UTIL_DEFER_HPP_INCLUDED_ 1

#include <util/macro.hpp>
#include <util/type_traits.hpp>

#include <concepts>
#include <exception>
#include <utility>

namespace util {

template <typename T>
concept suitable_for_defer =
    ::std::is_nothrow_move_constructible_v<T> &&
    ::std::is_nothrow_destructible_v<T> &&
    ::std::is_invocable_v<T &&> &&
    ::std::is_void_v<::std::invoke_result_t<T &&>>;

/* RAII class for performing an action at the end of a scope */

template <suitable_for_defer T>
class [[nodiscard]] defer final {
 private:
  UTIL_NO_UNIQUE_ADDRESS T action_;

 public:
  constexpr ~defer() noexcept (::std::is_nothrow_invocable_v<T &&>) {
    ::std::forward<T>(action_)();
  }

  defer(defer const &) = delete;
  void operator= (defer const &) = delete;

  defer(defer &&) = delete;
  void operator= (defer &&) = delete;

 public:
  constexpr explicit defer(T action) noexcept
			: action_(::std::forward<T>(action)) {}
};

////////////////////////////////////////////////////////////////////////////////

template <typename Policy>
concept scope_guard_policy = requires (Policy const &p) {
	requires ::std::is_class_v<Policy>;
	requires ::std::is_nothrow_default_constructible_v<Policy>;
	requires ::std::is_nothrow_destructible_v<Policy>;

	{ p.check() } noexcept -> ::std::same_as<bool>;
};

struct default_guard_policy {
	[[nodiscard]] constexpr bool check() const noexcept {
		return true;
	}
};

static_assert(scope_guard_policy<default_guard_policy>);

/** RAII class for performing an action at the end of a scope
 *  if it is active and the policy is met
 */

template <suitable_for_defer T,
					scope_guard_policy Policy = default_guard_policy>
class [[nodiscard]] scope_guard final : private Policy {
 private:
 	UTIL_NO_UNIQUE_ADDRESS T action_;
	bool active_; // [TODO]: State (active + used)

 public:
	constexpr ~scope_guard() noexcept (::std::is_nothrow_invocable_v<T &&>) {
		if (should_be_activated()) {
			::std::forward<T>(action_)();
		}
	}

	scope_guard(scope_guard const &) = delete;
	void operator= (scope_guard const &) = delete;

	scope_guard(scope_guard &&) = delete;
	void operator= (scope_guard &&) = delete;

 public:
	constexpr explicit scope_guard(T action) noexcept
			: Policy()
			, action_(::std::forward<T>(action)) {}

 public:
	[[nodiscard]] constexpr bool should_be_activated() const noexcept {
		return active_ && Policy::check();
	}

	void enable() noexcept {
		active_ = true;
	}

	void disable() noexcept {
		active_ = false;
	}

	void activate_under_policy() &&
		  noexcept (::std::is_nothrow_invocable_v<T &&>) {
		if (Policy::check()) {
			active_ = false;
			::std::forward<T>(action_)();
		}
	}

	void activate_if_should_be() &&
			noexcept (::std::is_nothrow_invocable_v<T &&>) {
		if (should_be_activated()) {
			active_ = false;
			::std::forward<T>(action_)();
		}
	}
};

////////////////////////////////////////////////////////////////////////////////

/* Guards that are triggered only if there is or is not an exception */

class success_guard_policy {
 private:
	int uncaught_on_creating_ = ::std::uncaught_exceptions();

 public:
	[[nodiscard]] bool check() const noexcept {
		return ::std::uncaught_exceptions() <= uncaught_on_creating_;
	}
};

static_assert(scope_guard_policy<success_guard_policy>);

template <typename T>
using scope_success = scope_guard<T, success_guard_policy>;

class failure_guard_policy {
 private:
	int uncaught_on_creating_ = ::std::uncaught_exceptions();

 public:
	[[nodiscard]] bool check() const noexcept {
		return ::std::uncaught_exceptions() > uncaught_on_creating_;
	}
};

static_assert(scope_guard_policy<failure_guard_policy>);

template <typename T>
using scope_failure = scope_guard<T, failure_guard_policy>;

////////////////////////////////////////////////////////////////////////////////

/* Type deduction helpers */

template <typename Policy>
struct guard_with_policy {
	template <typename T>
	using type = scope_guard<T, Policy>;
};

} // namespace util

#endif /* DDVAMP_UTIL_DEFER_HPP_INCLUDED_ */
