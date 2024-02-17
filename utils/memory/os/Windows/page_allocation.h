// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

// This file is for internal use and is not intended for direct inclusion

#include <cstddef>

namespace utils {

namespace {

constexpr ::std::size_t kPageSize = 4096;

::std::size_t get_page_size() noexcept
{
	return kPageSize;
}

void *allocate_memory(::std::size_t size)
{
	return ::operator new (size);
}

void protect_memory(void */* address */, ::std::size_t /* size */) noexcept
{
	// noop
}

void release_memory(void *address, ::std::size_t /* size */) noexcept
{
	::operator delete (address);
}

} // namespace

} // namespace utils
