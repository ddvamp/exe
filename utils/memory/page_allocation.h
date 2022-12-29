#ifndef DDV_UTILS_MEMORY_PAGE_ALLOCATION_H_
#define DDV_UTILS_MEMORY_PAGE_ALLOCATION_H_ 1

#include <cstddef>

#include "utils/memory/view.h"

namespace utils {

class page_allocation {
private:
	::std::byte *begin_ = nullptr;
	::std::size_t size_ = 0;

public:
	~page_allocation()
	{
		release();
	}

	page_allocation(page_allocation const &) = delete;
	void operator= (page_allocation const &) = delete;

	page_allocation(page_allocation &&that) noexcept
	{
		steal(that);
	}
	page_allocation &operator= (page_allocation &&that) noexcept
	{
		release();
		steal(that);
		return *this;
	}

public:
	page_allocation() = default;

	::std::byte *begin() const noexcept
	{
		return begin_;
	}

	::std::byte *end() const noexcept
	{
		return begin_ + size_;
	}

	::std::size_t size() const noexcept
	{
		return size_;
	}

	memory_view view() const noexcept
	{
		return {begin_, size_};
	}

	static ::std::size_t page_size() noexcept;

	static page_allocation allocate_pages(::std::size_t count) noexcept;

	void protect_pages(::std::size_t page_offset,
		::std::size_t page_count) const noexcept;

private:
	page_allocation(::std::byte *begin, ::std::size_t size) noexcept
		: begin_(begin)
		, size_(size)
	{}

	void reset() noexcept
	{
		begin_ = nullptr;
		size_ = 0;
	}

	void steal(page_allocation &that) noexcept
	{
		begin_ = that.begin_;
		size_ = that.size_;
		that.reset();
	}

	void release() const noexcept;
};

} // namespace utils

#endif /* DDV_UTILS_MEMORY_PAGE_ALLOCATION_H_ */
