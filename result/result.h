// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_RESULT_RESULT_H_
#define DDV_RESULT_RESULT_H_ 1

#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>

#include "utils/debug.h"
#include "utils/defer.h"
#include "utils/type_traits.h"
#include "utils/utility.h"

namespace utils {

namespace detail {

template <typename F, typename ...Args>
using result_ = ::std::remove_cvref_t<::std::invoke_result_t<F, Args...>>;

} // namespace detail


////////////////////////////////////////////////////////////////////////////////


using error = ::std::exception_ptr;

class bad_result_access : public ::std::exception {
public:
	[[nodiscard]] char const *what() const noexcept override
	{
		return "utils::result does not contain a value, as well as an error";
	}
};


////////////////////////////////////////////////////////////////////////////////


struct value_place_t {
	explicit value_place_t() = default;
};

inline constexpr value_place_t value_place{};

struct invoke_place_t {
	explicit invoke_place_t() = default;
};

inline constexpr invoke_place_t invoke_place{};


////////////////////////////////////////////////////////////////////////////////


template <typename T>
concept suitable_non_void_for_result =
	// Cpp17Destructible
	::std::is_object_v<T> &&
	!::std::is_array_v<T> &&
	::std::is_nothrow_destructible_v<T> &&

	!is_any_of_v<
		::std::remove_cv_t<T>,
		error,
		value_place_t,
		invoke_place_t
	>;

template <typename T>
concept suitable_for_result =
	suitable_non_void_for_result<T> ||
	::std::is_void_v<T>;


////////////////////////////////////////////////////////////////////////////////


// result contains value or error
//
// T must be a type other than
// error, result_place_t or invoke_place_t that meets
// the Cpp17Destructible requirements, or be cv void
template <suitable_for_result T>
class [[nodiscard]] result;


////////////////////////////////////////////////////////////////////////////////


template <typename T>
inline constexpr bool is_result_v =
	is_specialization_v<::std::remove_cv_t<T>, result>;

template <typename T>
struct is_result : ::std::bool_constant<is_result_v<T>> {};


////////////////////////////////////////////////////////////////////////////////


template <suitable_for_result T>
class [[nodiscard]] result {
private:
	using E = utils::error;

	union {
		T value_;
		E error_;
	};
	bool is_ok_;

	template <suitable_for_result>
	friend class result;

public:
	using value_type = T;

public:
	~result() noexcept
	{
		if (is_ok_) {
			if constexpr (!::std::is_trivially_destructible_v<T>) {
				value_.~T();
			}
		} else {
			error_.~E();
		}
	}

	result(result const &that)
		noexcept (::std::is_nothrow_copy_constructible_v<T>)
		requires (::std::is_copy_constructible_v<T>)
		: is_ok_(that.is_ok_)
	{
		if (is_ok_) {
			::new (::std::addressof(value_)) T(that.value_);
		} else {
			::new (::std::addressof(error_)) E(that.error_);
		}
	}
	result &operator= (result const &that)
		noexcept (
			::std::is_nothrow_copy_constructible_v<T> &&
			::std::is_nothrow_copy_assignable_v<T>
		)
		requires (
			::std::is_copy_assignable_v<T> &&
			::std::is_copy_constructible_v<T>
		)
	{
		if (is_ok_ != that.is_ok_) {
			if (is_ok_) {
				if constexpr (!::std::is_trivially_destructible_v<T>) {
					value_.~T();
				}
				::new (::std::addressof(error_)) E(that.error_);
			} else {
				reinit_value(that.value_);
			}

			is_ok_ = that.is_ok_;
		} else {
			if (is_ok_) {
				value_ = that.value_;
			} else {
				error_ = that.error_;
			}
		}

		return *this;
	}

	result(result &&that)
		noexcept (::std::is_nothrow_move_constructible_v<T>)
		requires (::std::is_move_constructible_v<T>)
		: is_ok_(that.is_ok_)
	{
		if (is_ok_) {
			::new (::std::addressof(value_)) T(::std::move(that.value_));
		} else {
			::new (::std::addressof(error_)) E(::std::move(that.error_));
		}
	}
	result &operator= (result &&that)
		noexcept (
			::std::is_nothrow_move_constructible_v<T> &&
			::std::is_nothrow_move_assignable_v<T>
		)
		requires (
			::std::is_move_assignable_v<T> &&
			::std::is_move_constructible_v<T>
		)
	{
		if (is_ok_ != that.is_ok_) {
			if (!is_ok_) {
				if constexpr (!::std::is_trivially_destructible_v<T>) {
					value_.~T();
				}
				::new (::std::addressof(error_)) E(::std::move(that.error_));
			} else {
				reinit_value(::std::move(that.value_));
			}

			is_ok_ = that.is_ok_;
		} else {
			if (is_ok_) {
				value_ = ::std::move(that.value_);
			} else {
				error_ = ::std::move(that.error_);
			}
		}

		return *this;
	}

public:
	result()
		noexcept (::std::is_nothrow_default_constructible_v<T>)
		requires (::std::is_default_constructible_v<T>)
		: value_()
		, is_ok_(true)
	{}

	template <typename U>
	inline static constexpr bool allow_unwrapping =
		!::std::is_constructible_v<T, result<U> &> &&
		!::std::is_constructible_v<T, result<U> const &> &&
		!::std::is_constructible_v<T, result<U>> &&
		!::std::is_constructible_v<T, result<U> const> &&
		!::std::is_convertible_v<result<U> &, T> &&
		!::std::is_convertible_v<result<U> const &, T> &&
		!::std::is_convertible_v<result<U>, T> &&
		!::std::is_convertible_v<result<U> const, T>;


	template <typename U>
	explicit (!::std::is_convertible_v<U const &, T>)
	result(result<U> const &that)
		noexcept (::std::is_nothrow_constructible_v<T, U const &>)
		requires (
			::std::is_constructible_v<T, U const &> &&
			allow_unwrapping<U>
		)
		: result(that.is_ok_ ? result(that.value_) : result(that.error_))
	{}

	template <typename U>
	explicit (!::std::is_convertible_v<U, T>) result(result<U> &&that)
		noexcept (::std::is_nothrow_constructible_v<T, U>)
		requires (
			::std::is_constructible_v<T, U> &&
			allow_unwrapping<U>
		)
		: result(
			that.is_ok_ ?
			result(::std::move(that.value_)) :
			result(::std::move(that.error_))
		)
	{}

	template <typename U = T>
	explicit (!::std::is_convertible_v<U, T>) result(U &&value)
		noexcept (::std::is_nothrow_constructible_v<T, U>)
		requires (
			!is_any_of_v<
				::std::remove_cvref_t<U>,
				result,
				E,
				value_place_t,
				invoke_place_t
			> &&
			::std::is_constructible_v<T, U>
		)
		: value_(::std::forward<U>(value))
		, is_ok_(true)
	{}

	result(E error) noexcept
		: error_(::std::move(error))
		, is_ok_(false)
	{}

	template <typename ...Args>
	explicit result(value_place_t, Args &&...args)
		noexcept (::std::is_nothrow_constructible_v<T, Args...>)
		requires (::std::is_constructible_v<T, Args...>)
		: value_(::std::forward<Args>(args)...)
		, is_ok_(true)
	{}

	template <typename U, typename ...Args>
	explicit result(value_place_t, ::std::initializer_list<U> ilist,
		Args &&...args)
		noexcept (
			::std::is_nothrow_constructible_v<
				T,
				::std::initializer_list<U> &,
				Args...
			>
		)
		requires (
			::std::is_constructible_v<T, ::std::initializer_list<U> &, Args...>
		)
		: value_(ilist, ::std::forward<Args>(args)...)
		, is_ok_(true)
	{}

	template <typename F, typename ...Args>
	explicit result(invoke_place_t, F &&f, Args &&...args)
		noexcept (is_nothrow_constructible_with_v<T, F, Args...>)
		requires (is_constructible_with_v<T, F, Args...>)
		: value_(
			::std::invoke(::std::forward<F>(f), ::std::forward<Args>(args)...)
		)
		, is_ok_(true)
	{}

	template <typename U = T>
	result &operator= (U &&value)
		noexcept (
			::std::is_nothrow_constructible_v<T, U> &&
			::std::is_nothrow_assignable_v<T &, U>
		)
		requires (
			!is_any_of_v<
				::std::remove_cvref_t<U>,
				result,
				error
			> &&
			::std::is_constructible_v<T, U> &&
			::std::is_assignable_v<T &, U>
		)
	{
		if (is_ok_) {
			value_ = ::std::forward<U>(value);
		} else {
			reinit_value(::std::forward<U>(value));
			is_ok_ = true;
		}

		return *this;
	}

	result &operator= (E error) noexcept
	{
		if (is_ok_) {
			if constexpr (!::std::is_trivially_destructible_v<T>) {
				value_.~T();
			}
			::new (::std::addressof(error_)) E(::std::move(error));
		} else {
			error_ = ::std::move(error);
		}

		return *this;
	}

	template <typename ...Args>
	T &emplace(Args &&...args) noexcept
		requires (::std::is_nothrow_constructible_v<T, Args...>)
	{
		if (is_ok_) {
			if constexpr (!::std::is_trivially_destructible_v<T>) {
				value_.~T();
			}
		} else {
			error_.~E();
			is_ok_ = true;
		}

		return *::new (::std::addressof(value_)) T(
			::std::forward<Args>(args)...
		);
	}

	template <typename U, typename ...Args>
	T &emplace(::std::initializer_list<U> ilist, Args &&...args) noexcept
		requires (
			::std::is_nothrow_constructible_v<
				T,
				::std::initializer_list<U>,
				Args...
			>
		)
	{
		if (is_ok_) {
			if constexpr (!::std::is_trivially_destructible_v<T>) {
				value_.~T();
			}
		} else {
			error_.~E();
			is_ok_ = true;
		}

		return *::new (::std::addressof(value_)) T(
			ilist,
			::std::forward<Args>(args)...
		);
	}

	void swap(result &that)
		noexcept (
			::std::is_nothrow_swappable_v<T> &&
			::std::is_nothrow_move_constructible_v<T>
		)
		requires (
			::std::is_swappable_v<T> &&
			::std::is_move_constructible_v<T>
		)
	{
		if (is_ok_ != that.is_ok_) {
			if (!is_ok_) {
				reinit_swap(that);
			} else {
				that.reinit_swap(*this);
			}
			is_ok_ = !is_ok_;
			that.is_ok_ = !that.is_ok_;
		} else {
			using ::std::swap;
			if (is_ok_) {
				swap(value_, that.value_);
			} else {
				swap(error_, that.error_);
			}
		}
	}

	friend void swap(result &lhs, result &rhs)
		noexcept (
			::std::is_nothrow_swappable_v<T> &&
			::std::is_nothrow_move_constructible_v<T>
		)
		requires (
			::std::is_swappable_v<T> &&
			::std::is_move_constructible_v<T>
		)
	{
		lhs.swap(rhs);
	}

	// for suppress [[nodiscard]]
	void ignore() const noexcept
	{
		// nothing
	}

	[[nodiscard]] T const *operator-> () const noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
		return ::std::addressof(value_);
	}

	[[nodiscard]] T *operator-> () noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
		return ::std::addressof(value_);
	}

	[[nodiscard]] T const &operator* () const & noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
		return value_;
	}

	[[nodiscard]] T &operator* () & noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
		return value_;
	}

	[[nodiscard]] T const &&operator* () const && noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
		return ::std::move(value_);
	}

	[[nodiscard]] T &&operator* () && noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
		return ::std::move(value_);
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return is_ok_;
	}

	[[nodiscard]] bool is_ok() const noexcept
	{
		return is_ok_;
	}

	[[nodiscard]] bool has_value() const noexcept
	{
		return is_ok_;
	}

	[[nodiscard]] bool has_error() const noexcept
	{
		return !is_ok_;
	}

	[[nodiscard]] T const &value() const &
	{
		throw_if_error();

		return value_;
	}

	[[nodiscard]] T &value() &
	{
		throw_if_error();

		return value_;
	}

	[[nodiscard]] T const &&value() const &&
	{
		throw_if_error();

		return ::std::move(value_);
	}

	[[nodiscard]] T &&value() &&
	{
		throw_if_error();

		return ::std::move(value_);
	}

	[[nodiscard]] E const &error() const & noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return error_;
	}

	[[nodiscard]] E &error() & noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return error_;
	}

	[[nodiscard]] E const &&error() const && noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return ::std::move(error_);
	}

	[[nodiscard]] E &&error() && noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return ::std::move(error_);
	}

	template <typename U>
	[[nodiscard]] T value_or(U &&value) const &
		noexcept (
			::std::is_nothrow_copy_constructible_v<T> &&
			::std::is_nothrow_convertible_v<U, T>
		)
		requires (
			::std::is_copy_constructible_v<T> &&
			::std::is_convertible_v<U, T>
		)
	{
		if (is_ok_) {
			return value_;
		} else {
			return static_cast<T>(::std::forward<U>(value));
		}
	}

	template <typename U>
	[[nodiscard]] T value_or(U &&value) &&
		noexcept (
			::std::is_nothrow_move_constructible_v<T> &&
			::std::is_nothrow_convertible_v<U, T>
		)
		requires (
			::std::is_move_constructible_v<T> &&
			::std::is_convertible_v<U, T>
		)
	{
		if (is_ok_) {
			return ::std::move(value_);
		} else {
			return static_cast<T>(::std::forward<U>(value));
		}
	}

	template <typename F>
	auto and_then(F &&f) &
		noexcept (::std::is_nothrow_invocable_v<F, T &>)
		requires (
			::std::is_invocable_v<F, T &> &&
			is_result_v<detail::result_<F, T &>>
		)
	{
		using Res = detail::result_<F, T &>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f), value_)) :
			Res(error_);
	}

	template <typename F>
	auto and_then(F &&f) const &
		noexcept (::std::is_nothrow_invocable_v<F, T const &>)
		requires (
			::std::is_invocable_v<F, T const &> &&
			is_result_v<detail::result_<F, T const &>>
		)
	{
		using Res = detail::result_<F, T const &>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f), value_)) :
			Res(error_);
	}

	template <typename F>
	auto and_then(F &&f) &&
		noexcept (::std::is_nothrow_invocable_v<F, T>)
		requires (
			::std::is_invocable_v<F, T> &&
			is_result_v<detail::result_<F, T>>
		)
	{
		using Res = detail::result_<F, T>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f), ::std::move(value_))) :
			Res(::std::move(error_));
	}

	template <typename F>
	auto and_then(F &&f) const &&
		noexcept (::std::is_nothrow_invocable_v<F, T const>)
		requires (
			::std::is_invocable_v<F, T const> &&
			is_result_v<detail::result_<F, T const>>
		)
	{
		using Res = detail::result_<F, T const>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f), ::std::move(value_))) :
			Res(::std::move(error_));
	}

	template <typename F>
	auto or_else(F &&f) &
		noexcept (
			::std::is_nothrow_copy_constructible_v<T> &&
			::std::is_nothrow_invocable_v<F, E &>
		)
		requires (
			::std::is_copy_constructible_v<T> &&
			::std::is_invocable_v<F, E &> &&
			::std::is_same_v<detail::result_<F, E &>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res(value_place, value_) :
			Res(::std::invoke(::std::forward<F>(f), error_));
	}

	template <typename F>
	auto or_else(F &&f) const &
		noexcept (
			::std::is_nothrow_copy_constructible_v<T> &&
			::std::is_nothrow_invocable_v<F, E const &>
		)
		requires (
			::std::is_copy_constructible_v<T> &&
			::std::is_invocable_v<F, E const &> &&
			::std::is_same_v<detail::result_<F, E const &>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res(value_place, value_) :
			Res(::std::invoke(::std::forward<F>(f), error_));
	}

	template <typename F>
	auto or_else(F &&f) &&
		noexcept (
			::std::is_nothrow_move_constructible_v<T> &&
			::std::is_nothrow_invocable_v<F, E>
		)
		requires (
			::std::is_move_constructible_v<T> &&
			::std::is_invocable_v<F, E> &&
			::std::is_same_v<detail::result_<F, E>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res(value_place, ::std::move(value_)) :
			Res(::std::invoke(::std::forward<F>(f), ::std::move(error_)));
	}

	template <typename F>
	auto or_else(F &&f) const &&
		noexcept (
			::std::is_nothrow_move_constructible_v<T> &&
			::std::is_nothrow_invocable_v<F, E const>
		)
		requires (
			::std::is_move_constructible_v<T> &&
			::std::is_invocable_v<F, E const> &&
			::std::is_same_v<detail::result_<F, E const>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res(value_place, ::std::move(value_)) :
			Res(::std::invoke(::std::forward<F>(f), ::std::move(error_)));
	}

	template <typename F>
	auto transform(F &&f) &
		noexcept (::std::is_nothrow_invocable_v<F, T &>)
		requires (
			::std::is_invocable_v<F, T &> &&
			suitable_for_result<detail::result_<F, T &>>
		)
	{
		using R = detail::result_<F, T &>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f), value_) :
			Res(error_);
	}

	template <typename F>
	auto transform(F &&f) const &
		noexcept (::std::is_nothrow_invocable_v<F, T const &>)
		requires (
			::std::is_invocable_v<F, T const &> &&
			suitable_for_result<detail::result_<F, T const &>>
		)
	{
		using R = detail::result_<F, T const &>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f), value_) :
			Res(error_);
	}

	template <typename F>
	auto transform(F &&f) &&
		noexcept (::std::is_nothrow_invocable_v<F, T>)
		requires (
			::std::is_invocable_v<F, T> &&
			suitable_for_result<detail::result_<F, T>>
		)
	{
		using R = detail::result_<F, T>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f), ::std::move(value_)) :
			Res(::std::move(error_));
	}

	template <typename F>
	auto transform(F &&f) const &&
		noexcept (::std::is_nothrow_invocable_v<F, T const>)
		requires (
			::std::is_invocable_v<F, T const> &&
			suitable_for_result<detail::result_<F, T const>>
		)
	{
		using R = detail::result_<F, T const>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f), ::std::move(value_)) :
			Res(::std::move(error_));
	}

private:
	template <typename ...Args>
	void reinit_value(Args &&...args)
		noexcept (::std::is_nothrow_constructible_v<T, Args...>)
	{
		if constexpr (::std::is_nothrow_constructible_v<T, Args...>) {
			error_.~E();
			::new (::std::addressof(value_)) T(::std::forward<Args>(args)...);
		} else if constexpr (::std::is_nothrow_move_constructible_v<T>) {
			T tmp(::std::forward<Args>(args)...);
			error_.~E();
			::new (::std::addressof(value_)) T(::std::move(tmp));
		} else {
			auto rollback = scope_guard{
				[err = ::std::move(error_), where = ::std::addressof(error_)]
				() noexcept {
					::new (where) E(::std::move(err));
				}
			};

			error_.~E();
			::new (::std::addressof(value_)) T(::std::forward<Args>(args)...);
			rollback.disable();
		}
	}

	void reinit_swap(result &that)
		noexcept (::std::is_nothrow_move_constructible_v<T>)
	{
		E tmp(::std::move(error_));

		if constexpr (::std::is_nothrow_move_constructible_v<T>) {
			::new (::std::addressof(value_)) T(::std::move(that.value_));
		} else {
			auto rollback = scope_guard{
				[&tmp, where = ::std::addressof(error_)]() noexcept {
					::new (where) E(::std::move(tmp));
				}
			};

			::new (::std::addressof(value_)) T(::std::move(that.value_));
			rollback.disable();
		}

		if constexpr (!::std::is_trivially_destructible_v<T>) {
			that.value_.~T();
		}
		::new (::std::addressof(that.error_)) E(::std::move(tmp));
	}

	[[noreturn]] void throw_exception() const
	{
		if (error_) {
			::std::rethrow_exception(error_);
		} else {
			throw bad_result_access{};
		}
	}

	void throw_if_error() const
	{
		if (!is_ok_) {
			throw_exception();
		}
	}
};

template <typename F, typename ...Args>
result(invoke_place_t, F &&, Args &&...) -> result<detail::result_<F, Args...>>;


////////////////////////////////////////////////////////////////////////////////


template <suitable_for_result T>
requires (::std::is_void_v<T>)
class [[nodiscard]] result<T> {
private:
	using E = utils::error;

	union {
		E error_;
	};
	bool is_ok_;

	template <suitable_for_result>
	friend class result;

public:
	using value_type = T;

public:
	~result() noexcept
	{
		if (!is_ok_) {
			error_.~E();
		}
	}

	result(result const &that) noexcept
		: is_ok_(that.is_ok_)
	{
		if (!is_ok_) {
			::new (::std::addressof(error_)) E(that.error_);
		}
	}
	result &operator= (result const &that) noexcept
	{
		if (is_ok_ != that.is_ok_) {
			if (is_ok_) {
				::new (::std::addressof(error_)) E(that.error_);
			} else {
				error_.~E();
			}

			is_ok_ = that.is_ok_;
		} else {
			if (!is_ok_) {
				error_ = that.error_;
			}
		}

		return *this;
	}

	result(result &&that) noexcept
		: is_ok_(that.is_ok_)
	{
		if (!is_ok_) {
			::new (::std::addressof(error_)) E(::std::move(that.error_));
		}
	}
	result &operator= (result &&that) noexcept
	{
		if (is_ok_ != that.is_ok_) {
			if (!is_ok_) {
				::new (::std::addressof(error_)) E(::std::move(that.error_));
			} else {
				error_.~E();
			}

			is_ok_ = that.is_ok_;
		} else {
			if (!is_ok_) {
				error_ = ::std::move(that.error_);
			}
		}

		return *this;
	}

public:
	result() noexcept
		: is_ok_(true)
	{}

	template <typename U>
	explicit result(result<U> const &that) noexcept
		: result(that.is_ok_ ? result() : result(that.error_))
	{}

	template <typename U>
	explicit result(result<U> &&that) noexcept
		: result(that.is_ok_ ? result() : result(::std::move(that.error_)))
	{}

	result(E error) noexcept
		: error_(::std::move(error))
		, is_ok_(false)
	{}

	explicit result(value_place_t) noexcept
		: is_ok_(true)
	{}

	template <typename F, typename ...Args>
	result(invoke_place_t, F &&f, Args &&...args)
		noexcept (::std::is_nothrow_invocable_v<F, Args...>)
		requires (::std::is_invocable_v<F, Args...>)
		: is_ok_(true)
	{
		::std::invoke(::std::forward<F>(f), ::std::forward<Args>(args)...);
	}

	result &operator= (E error) noexcept
	{
		if (is_ok_) {
			::new (::std::addressof(error_)) E(::std::move(error));
		} else {
			error_ = ::std::move(error);
		}

		return *this;
	}

	void emplace() noexcept
	{
		if (!is_ok_) {
			error_.~E();
			is_ok_ = true;
		}
	}

	void swap(result &that) noexcept
	{
		if (is_ok_ != that.is_ok_) {
			if (!is_ok_) {
				::new (::std::addressof(that.error_)) E(::std::move(error_));
				error_.~E();
			} else {
				::new (::std::addressof(error_)) E(::std::move(that.error_));
				that.error_.~E();
			}
			is_ok_ = !is_ok_;
			that.is_ok_ = !that.is_ok_;
		} else if (!is_ok_) {
			using ::std::swap;
			swap(error_, that.error_);
		}
	}

	friend void swap(result &lhs, result &rhs) noexcept
	{
		lhs.swap(rhs);
	}

	// for suppress [[nodiscard]]
	void ignore() const noexcept
	{
		// nothing
	}

	void operator* () const noexcept
	{
		UTILS_ASSERT(is_ok_, "result stores an error");
	}

	[[nodiscard]] explicit operator bool() const noexcept
	{
		return is_ok_;
	}

	[[nodiscard]] bool is_ok() const noexcept
	{
		return is_ok_;
	}

	[[nodiscard]] bool has_value() const noexcept
	{
		return is_ok_;
	}

	[[nodiscard]] bool has_error() const noexcept
	{
		return !is_ok_;
	}

	void value() const
	{
		throw_if_error();
	}

	[[nodiscard]] E const &error() const & noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return error_;
	}

	[[nodiscard]] E &error() & noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return error_;
	}

	[[nodiscard]] E const &&error() const && noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return ::std::move(error_);
	}

	[[nodiscard]] E &&error() && noexcept
	{
		UTILS_ASSERT(!is_ok_, "result stores a value");
		return ::std::move(error_);
	}

	template <typename F>
	auto and_then(F &&f) &
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			is_result_v<detail::result_<F>>
		)
	{
		using Res = detail::result_<F>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f))) :
			Res(error_);
	}

	template <typename F>
	auto and_then(F &&f) const &
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			is_result_v<detail::result_<F>>
		)
	{
		using Res = detail::result_<F>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f))) :
			Res(error_);
	}

	template <typename F>
	auto and_then(F &&f) &&
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			is_result_v<detail::result_<F>>
		)
	{
		using Res = detail::result_<F>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f))) :
			Res(::std::move(error_));
	}

	template <typename F>
	auto and_then(F &&f) const &&
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			is_result_v<detail::result_<F>>
		)
	{
		using Res = detail::result_<F>;

		return is_ok_ ?
			Res(::std::invoke(::std::forward<F>(f))) :
			Res(::std::move(error_));
	}

	template <typename F>
	auto or_else(F &&f) &
		noexcept (::std::is_nothrow_invocable_v<F, E &>)
		requires (
			::std::is_invocable_v<F, E &> &&
			::std::is_same_v<detail::result_<F, E &>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res() :
			Res(::std::invoke(::std::forward<F>(f), error_));
	}

	template <typename F>
	auto or_else(F &&f) const &
		noexcept (::std::is_nothrow_invocable_v<F, E const &>)
		requires (
			::std::is_invocable_v<F, E const &> &&
			::std::is_same_v<detail::result_<F, E const &>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res() :
			Res(::std::invoke(::std::forward<F>(f), error_));
	}

	template <typename F>
	auto or_else(F &&f) &&
		noexcept (::std::is_nothrow_invocable_v<F, E>)
		requires (
			::std::is_invocable_v<F, E> &&
			::std::is_same_v<detail::result_<F, E>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res() :
			Res(::std::invoke(::std::forward<F>(f), ::std::move(error_)));
	}

	template <typename F>
	auto or_else(F &&f) const &&
		noexcept (::std::is_nothrow_invocable_v<F, E const>)
		requires (
			::std::is_invocable_v<F, E const> &&
			::std::is_same_v<detail::result_<F, E const>, result>
		)
	{
		using Res = result;

		return is_ok_ ?
			Res() :
			Res(::std::invoke(::std::forward<F>(f), ::std::move(error_)));
	}

	template <typename F>
	auto transform(F &&f) &
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			suitable_for_result<detail::result_<F>>
		)
	{
		using R = detail::result_<F>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f)) :
			Res(error_);
	}

	template <typename F>
	auto transform(F &&f) const &
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			suitable_for_result<detail::result_<F>>
		)
	{
		using R = detail::result_<F>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f)) :
			Res(error_);
	}

	template <typename F>
	auto transform(F &&f) &&
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			suitable_for_result<detail::result_<F>>
		)
	{
		using R = detail::result_<F>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f)) :
			Res(::std::move(error_));
	}

	template <typename F>
	auto transform(F &&f) const &&
		noexcept (::std::is_nothrow_invocable_v<F>)
		requires (
			::std::is_invocable_v<F> &&
			suitable_for_result<detail::result_<F>>
		)
	{
		using R = detail::result_<F>;
		using Res = result<R>;

		return is_ok_ ?
			Res(invoke_place, ::std::forward<F>(f)) :
			Res(::std::move(error_));
	}

private:
	[[noreturn]] void throw_exception() const
	{
		if (error_) {
			::std::rethrow_exception(error_);
		} else {
			throw bad_result_access{};
		}
	}

	void throw_if_error() const
	{
		if (!is_ok_) {
			throw_exception();
		}
	}
};

using status = result<void>;

} // namespace utils

#include "result/result.ipp"

#endif /* DDV_RESULT_RESULT_H_ */
