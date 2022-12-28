#ifndef DDV_UTILS_MEMORY_PAGE_ALLOCATION_H_
#define DDV_UTILS_MEMORY_PAGE_ALLOCATION_H_ 1

#include <cstddef>

namespace utils {

class PageAllocation {
private:
	void *start_ = nullptr;
	::std::size_t size_ = 0;

public:
	~PageAllocation()
	{
		release();
	}

	PageAllocation(PageAllocation const &) = delete;
	void operator= (PageAllocation const &) = delete;

	PageAllocation(PageAllocation &&other) noexcept
		: start_(other.start_)
		, size_(other.size_)
	{
		other.reset();
	}
	PageAllocation &operator= (PageAllocation &&other) noexcept
	{
		release();
		start_ = other.start_;
		size_ = other.size_;
		other.reset();
		return *this;
	}

public:
	PageAllocation() = default;

	void *start() const noexcept
	{
		return start_;
	}

	void *end() const noexcept
	{
		return static_cast<char *>(start_) + size_;
	}

	::std::size_t size() const noexcept
	{
		return size_;
	}

	static ::std::size_t pageSize() noexcept;

	static PageAllocation allocatePages(::std::size_t count) noexcept;

	void protectPages(::std::size_t page_offset,
		::std::size_t page_count) const noexcept;

private:
	PageAllocation(void *start, ::std::size_t size) noexcept
		: start_(start)
		, size_(size)
	{}

	void reset() noexcept
	{
		start_ = nullptr;
		size_ = 0;
	}

	void release() const noexcept;
};

} // namespace utils

#endif /* DDV_UTILS_MEMORY_PAGE_ALLOCATION_H_ */
