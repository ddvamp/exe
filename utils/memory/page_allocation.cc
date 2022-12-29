#include "utils/memory/page_allocation.h"

#if __has_include(<Windows.h>)
#	include "utils/memory/os/windows/page_allocation.h"
#elif __has_include(<unistd.h>)
#	include "utils/memory/os/posix/page_allocation.h"
#else
#	error "unsupported environment (not Windows nor POSIX-compliant)"
#endif

namespace utils {

::std::size_t page_allocation::page_size() noexcept
{
	static ::std::size_t const kPageSize = get_page_size();
	return kPageSize;
}

static ::std::size_t pages_to_bytes(::std::size_t count) noexcept
{
	return count * page_allocation::page_size();
}

page_allocation page_allocation::allocate_pages(::std::size_t count) noexcept
{
	auto size = pages_to_bytes(count);

	auto begin = allocate_memory(size);

	return page_allocation{static_cast<::std::byte *>(begin), size};
}

void page_allocation::protect_pages(::std::size_t page_offset,
	::std::size_t page_count) const noexcept
{
	protect_memory(
		begin_ + pages_to_bytes(page_offset),
		pages_to_bytes(page_count)
	);
}

void page_allocation::release() const noexcept
{
	if (begin_) {
		release_memory(begin_, size_);
	}
}

} // namespace utils
