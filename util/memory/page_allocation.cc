// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <exception>
#include <limits>

#include "util/debug.h"

#include "util/memory/page_allocation.h"

#if __has_include(<unistd.h>)
#	include "util/memory/os/posix/page_allocation.h"
#else
#	error "not POSIX-compliant environment"
#endif

namespace util {

/* static */ ::std::size_t page_allocation::page_size() noexcept
{
	static auto const kPageSize = get_page_size();
	return kPageSize;
}

/* static */ ::std::size_t page_allocation::max_pages() noexcept
{
	static auto const kMaxPages =
		(::std::numeric_limits<::std::size_t>::max)() /
		page_allocation::page_size();
	return kMaxPages;
}

static ::std::size_t pages_to_bytes(::std::size_t count) noexcept
{
	return count * page_allocation::page_size();
}

// precondition: count != 0 && count <= max_pages()
/* static */ page_allocation
	page_allocation::allocate_pages(::std::size_t count)
{
	UTILS_ASSERT(count != 0, "0 pages request");
	UTILS_ASSERT(count <= max_pages(), "too many pages");

	auto size = pages_to_bytes(count);

	auto begin = allocate_memory(size);

	return page_allocation{static_cast<::std::byte *>(begin), size};
}

// precondition: page_count != 0 &&
// protected memory in the range [begin_, begin_ + size_)
void page_allocation::protect_pages(::std::size_t page_offset,
	::std::size_t page_count) const
{
	UTILS_ASSERT(page_count != 0, "0 pages request");
	UTILS_ASSERT(page_count <= max_pages(), "out of range");
	UTILS_ASSERT(page_offset <= max_pages() - page_count , "out of range");
	UTILS_ASSERT(
		pages_to_bytes(page_offset + page_count) <= size_,
		"out of range"
	);

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

} // namespace util
