// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_RING_BUFFER_H_
#define DDV_UTILS_RING_BUFFER_H_

#include <vector>

#include <bit>
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

#include "utils/macro.h"

namespace utils {

template <typename T, typename Allocator = ::std::allocator<T>>
	requires ::std::same_as<T, typename Allocator::value_type>
class RingBuffer {
private:
	::std::size_t first_ = 0;
	::std::size_t last_ = 0;
	::std::size_t capacity_;

	UTILS_NO_UNIQUE_ADDRESS Allocator allocator_;

	::std::size_t data_size_ = ::std::bit_ceil(capacity_);
	T *data_ = allocator_.allocate(data_size_);

public:
	~RingBuffer()
	{
		while (!empty()) {
			pop_front();
		}

		allocator_.deallocate(data_, data_size_);
	}

	RingBuffer(RingBuffer const &) = delete;
	void operator= (RingBuffer const &) = delete;

	RingBuffer(RingBuffer &&) = delete;
	void operator= (RingBuffer &&) = delete;

public:
	explicit RingBuffer(::std::size_t capacity)
		: capacity_(capacity)
	{}

	[[nodiscard]] ::std::size_t capacity() const noexcept
	{
		return capacity_;
	}

	[[nodiscard]] ::std::size_t size() const noexcept
	{
		return last_ - first_;
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return size() == 0;
	}

	[[nodiscard]] bool full() const noexcept
	{
		return size() == capacity();
	}

	[[nodiscard]] T &front() noexcept
	{
		return data_[first_ % data_size_];
	}

	void push_back(T value)
	{
		::std::ranges::construct_at(
			data_ + last_++ % data_size_,
			::std::move(T)
		);
	}

	void pop_front()
	{
		::std::ranges::destroy_at(data_ + first_++ % data_size_);
	}
};

} // namespace utils

#endif /* DDV_UTILS_RING_BUFFER_H_ */
