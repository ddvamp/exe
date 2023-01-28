// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_FUNCTION_H_
#define DDV_UTILS_FUNCTION_H_ 1

#include <type_traits>
#include <utility>

#include "utils/abort.h"

namespace utils {

namespace detail {

// for uninitialized/empty function
template <typename R, typename ...Args>
[[noreturn]] R function_not_callable(void *, Args &&...) noexcept
{
	abort("empty utils::function object was called");
}

// precondition: ptr point to object of type T
template <typename T>
static constexpr void deleter(void* ptr) noexcept
{
	::delete static_cast<T *>(ptr);
}

template <typename R, bool nothrow, typename ...Args>
class FunctionImpl {
protected:
	using InvokerType = R (*)(void *, Args &&...) noexcept (nothrow);
	using DeleterType = void (*)(void *) noexcept;
	using FunctorType = void *;

	InvokerType invoker_;
	DeleterType deleter_;
	FunctorType functor_;

protected:
	~FunctionImpl() noexcept
	{
		release();
	}

	FunctionImpl(FunctionImpl const &) = delete;
	void operator= (FunctionImpl const &) = delete;

	FunctionImpl(FunctionImpl &&that) noexcept
	{
		steal(that);
	}

	FunctionImpl &operator= (FunctionImpl &&that) noexcept
	{
		if (this != &that) {
			release();
			steal(that);
		}

		return *this;
	}

protected:
	template <typename T>
	static constexpr R invoker(void *ptr, Args &&...args)
		noexcept (nothrow)
	{
		return (*static_cast<T *>(ptr))(::std::forward<Args>(args)...);
	}

	template <typename T, typename U = ::std::remove_cvref_t<T>>
	inline static constexpr bool enable_constructor =
		::std::is_class_v<U> &&
		::std::is_nothrow_destructible_v<U> &&
		::std::is_constructible_v<U, T> &&
		(
			nothrow ?
			::std::is_nothrow_invocable_r_v<R, U &, Args...> :
			::std::is_invocable_r_v<R, U &, Args...>
		);

	bool empty() const noexcept
	{
		return functor_;
	}

	FunctionImpl() noexcept
		: invoker_(&function_not_callable<R, Args...>)
		, deleter_(nullptr)
		, functor_(nullptr)
	{}

	R operator() (Args &&...args) const noexcept (nothrow)
	{
		return invoker_(functor_, ::std::forward<Args>(args)...);
	}

	void clear() noexcept
	{
		release();
		reset();
	}

	explicit operator bool() const noexcept
	{
		return !empty();
	}

	template <typename Fn> 
	void construct_fn(Fn &&fn)
	{
		using T = ::std::remove_cvref_t<Fn>;

		functor_ = ::new T(::std::forward<Fn>(f));
		invoker_ = invoker<T>;
		deleter_ = deleter<T>;
	}

	void steal(FunctionImpl &that) noexcept
	{
		invoker_ = that.invoker_;
		deleter_ = that.deleter_;
		functor_ = that.functor_;
		that.reset();
	}

	void reset() noexcept
	{
		invoker_ = &function_not_callable<R, Args...>;
		deleter_ = nullptr;
		functor_ = nullptr;
	}

	void release() noexcept
	{
		if (!empty()) {
			deleter_(functor_);
		}
	}
};

template <typename>
struct get_function_impl {
	static_assert(
		false,
		"utils::function expect function type as template argument"
	);
};

template <typename R, typename ...Args>
struct get_function_impl<R(Args...)> {
	using type = FunctionImpl<R, false, Args...>;
};

template <typename R, typename ...Args>
struct get_function_impl<R(Args...) noexcept> {
	using type = FunctionImpl<R, true, Args...>;
};

} // namespace utils::detail

// wrapper for functors whose operator() is of type FuncType
template <typename FuncType>
class function : private detail::get_function_impl<FuncType>::type {
private:
	using Base = typename detail::get_function_impl<FuncType>::type;

public:
	~function() = default;

	function(function const &) = delete;
	void operator= (function const &) = delete;

	function(function &&) = default;
	function &operator= (function &&) = default;

public:
	template <typename Fn>
	function(Fn &&fn)
		requires (Base::template enable_constructor<Fn>)
	{
		Base::construct_fn(::std::forward<Fn>(fn));
	}

	// uninitialized function
	function() = default;

	using Base::clear;
	using Base::operator bool;
	using Base::operator();
};

} // namespace utils

#endif /* DDV_UTILS_FUNCTION_H_ */
