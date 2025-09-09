//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

// This file is for internal use and is not intended for direct inclusion

#include <cerrno>
#include <cstddef>
#include <exception>

#include <sys/mman.h>
#include <unistd.h>

#include "util/debug.hpp"

namespace util {

namespace {

::std::size_t get_page_size() noexcept
{
	auto ret = ::sysconf(_SC_PAGE_SIZE);

	return static_cast<::std::size_t>(ret);
}

void *allocate_memory(::std::size_t size)
{
	auto memory = ::mmap(
		NULL,
		size,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1,
		0
	);

	if (memory == MAP_FAILED) [[unlikely]] {
		switch (errno) {
		// size is 0, too large, or not aligned on a page boundary
		case EINVAL:

		// no memory is available,
		// the process's maximum number of mappings would have been exceeded,
		// or the process's RLIMIT_DATA limit would have been exceeded
		case ENOMEM:
			throw std::bad_alloc{};

		default:
			UTIL_UNREACHABLE("mmap memory allocation error");
		}
	}

	return memory;
}

void protect_memory(void *address, ::std::size_t size) noexcept
{
	auto ret = ::mprotect(address, size, PROT_NONE);

	if (ret != 0) [[unlikely]] {
		switch (errno) {
		// internal kernel structures could not be allocated,
		// or changing the protection of a memory region would result in
		// the total number of mappings with distinct attributes
		case ENOMEM:
			throw std::bad_alloc{};

		default:
			UTIL_UNREACHABLE("mprotect memory protect error");
		}
	}
}

void release_memory(void *address, ::std::size_t size) noexcept
{
	[[maybe_unused]] auto ret = ::munmap(address, size);

	UTIL_ASSERT(ret == 0, "munmap memory release error");
}

} // namespace

} // namespace util
