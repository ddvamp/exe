#include "utils/memory/page_allocation.h"

#if __has_include(<Windows.h>)
#	include "utils/memory/os/windows/page_allocation.h"
#elif __has_include(<unistd.h>)
#	include "utils/memory/os/posix/page_allocation.h"
#else
#	error "unsupported environment (not Windows nor POSIX-compliant)"
#endif

namespace utils {

::std::size_t PageAllocation::pageSize() noexcept
{
	static ::std::size_t const kPageSize = getPageSize();
	return kPageSize;
}

static ::std::size_t pagesToBytes(::std::size_t count) noexcept
{
	return count * PageAllocation::pageSize();
}

PageAllocation PageAllocation::allocatePages(::std::size_t count) noexcept
{
	auto size = pagesToBytes(count);

	auto start = allocateMemory(size);

	return PageAllocation{start, size};
}

void PageAllocation::protectPages(::std::size_t page_offset,
	::std::size_t page_count) const noexcept
{
	protectMemory(
		static_cast<char *>(start_) + pagesToBytes(page_offset),
		pagesToBytes(page_count)
	);
}

void PageAllocation::release() const noexcept
{
	if (start_) {
		releaseMemory(start_, size_);
	}
}

} // namespace utils
