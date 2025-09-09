// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTIL_REFER_REF_COUNTED_PTR_H_
#define DDV_UTIL_REFER_REF_COUNTED_PTR_H_ 1

#include <cstddef>
#include <functional>
#include <utility>

#include "util/macro.hpp"
#include "util/refer/ref_counted.hpp"

namespace util {

template <typename T>
class RefCountedPtr {
private:
	T *ptr_ = nullptr;

	UTIL_NO_UNIQUE_ADDRESS detail::RefValidator<T> v_;

public:
	constexpr ~RefCountedPtr() noexcept
	{
		decRef();
	}

	RefCountedPtr(RefCountedPtr const &that) noexcept
		: ptr_(that.ptr_)
	{
		incRef();
	}

	RefCountedPtr(RefCountedPtr &&that) noexcept
	{
		that.swap(*this);
	}

	RefCountedPtr &operator= (RefCountedPtr that) noexcept
	{
		that.swap(*this);
		return *this;
	}

public:
	constexpr RefCountedPtr() noexcept = default;

	constexpr RefCountedPtr(::std::nullptr_t) noexcept
	{}

	constexpr explicit RefCountedPtr(T *ptr) noexcept
		: ptr_(ptr)
	{}

	[[nodiscard]] auto useCount() const noexcept
	{
		return ptr_ ? ptr_->useCount() : 0;
	}

	void swap(RefCountedPtr &that) noexcept
	{
		::std::swap(ptr_, that.ptr_);
	}

	void reset() noexcept
	{
		RefCountedPtr().swap(*this);
	}

	void reset(T *ptr) noexcept
	{
		RefCountedPtr(ptr).swap(*this);
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
[[nodiscard]] bool operator== (RefCountedPtr<T> const &lhs,
	RefCountedPtr<T> const &rhs) noexcept
{
	return ::std::ranges::equal_to{}(lhs.get(), rhs.get());
}

template <typename T>
[[nodiscard]] auto operator<=> (RefCountedPtr<T> const &lhs,
	RefCountedPtr<T> const &rhs) noexcept
{
	return ::std::compare_three_way{}(lhs.get(), rhs.get());
}

template <typename T>
void swap(RefCountedPtr<T> &lhs, RefCountedPtr<T> &rhs) noexcept
{
	lhs.swap(rhs);
}

} // namespace util

#endif /* DDV_UTIL_REFER_REF_COUNTED_PTR_H_ */
