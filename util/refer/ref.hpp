//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_
#define DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_ 1

#include <cstddef>
#include <functional>
#include <utility>

#include "util/macro.hpp"
#include "util/refer/ref_count.hpp"

namespace util {

template <typename T>
class Ref {
private:
	T *ptr_ = nullptr;

	UTIL_NO_UNIQUE_ADDRESS detail::RefValidator<T> v_;

public:
	constexpr ~Ref() noexcept
	{
		decRef();
	}

	Ref(Ref const &that) noexcept
		: ptr_(that.ptr_)
	{
		incRef();
	}

	Ref(Ref &&that) noexcept
	{
		that.swap(*this);
	}

	Ref &operator= (Ref that) noexcept
	{
		that.swap(*this);
		return *this;
	}

public:
	constexpr Ref() noexcept = default;

	constexpr Ref(::std::nullptr_t) noexcept
	{}

	constexpr explicit Ref(T *ptr) noexcept
		: ptr_(ptr)
	{}

	[[nodiscard]] auto useCount() const noexcept
	{
		return ptr_ ? ptr_->useCount() : 0;
	}

	void swap(Ref &that) noexcept
	{
		::std::swap(ptr_, that.ptr_);
	}

	void reset() noexcept
	{
		Ref().swap(*this);
	}

	void reset(T *ptr) noexcept
	{
		Ref(ptr).swap(*this);
	}

	[[nodiscard]] T *get() const noexcept
	{
		return ptr_;
	}

	[[nodiscard]] T &operator* () const noexcept
	{
		return *get();
	}

	[[nodiscard]] T *operator-> () const noexcept
	{
		return get();
	}

	explicit operator bool() const noexcept
	{
		return ptr_;
	}

private:
	void incRef() const noexcept
	{
		if (ptr_) {
			ptr_->incRef();
		}
	}

	void decRef() const noexcept
	{
		if (ptr_) {
			ptr_->decRef();
		}
	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
[[nodiscard]] bool operator== (Ref<T> const &lhs,
	Ref<T> const &rhs) noexcept
{
	return ::std::ranges::equal_to{}(lhs.get(), rhs.get());
}

template <typename T>
[[nodiscard]] auto operator<=> (Ref<T> const &lhs,
	Ref<T> const &rhs) noexcept
{
	return ::std::compare_three_way{}(lhs.get(), rhs.get());
}

template <typename T>
void swap(Ref<T> &lhs, Ref<T> &rhs) noexcept
{
	lhs.swap(rhs);
}

} // namespace util

#endif /* DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_ */
