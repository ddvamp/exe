//
// page_allocation.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <util/debug/assert.hpp>
#include <util/memory/page_allocation.hpp>

#if __has_include(<unistd.h>)
# include <internal/os/posix/page_allocation.hpp>
#else
# error "Not POSIX-compliant environment"
#endif

#include <cstddef>

namespace util {

// https://lxadm.com/why-are-page-sizes-always-powers-of-2/
/* static */ ::std::size_t page_allocation::page_size() noexcept {
  static auto const kPageSize = get_page_size();
  return kPageSize;
}

/* static */ ::std::size_t page_allocation::max_pages() noexcept {
  static auto const kMaxPages = (::std::size_t{} - 1) / page_size();
  return kMaxPages;
}

/* static */ ::std::size_t page_allocation::pages_to_bytes(
    ::std::size_t const page_count) noexcept {
  return page_count * page_size();
}

/* static */ ::std::size_t page_allocation::bytes_to_pages(
    ::std::size_t const at_least) noexcept {
  return at_least / page_size() + (at_least % page_size() != 0 ? 1 : 0);
}

/* static */ page_allocation page_allocation::allocate_pages(
    ::std::size_t const count) {
  auto const size = pages_to_bytes(count);
  auto const memory = allocate_memory(size);
  return page_allocation(::new (memory) ::std::byte[size], size);
}

// Precondition: page_count != 0 &&
//               protected memory in the range [begin_, begin_ + size_)
void page_allocation::protect_pages(::std::size_t const page_offset,
                                    ::std::size_t const page_count) {
  UTIL_ASSERT(page_count != 0, "0 pages requested");
  UTIL_ASSERT(page_count <= max_pages(), "Out of range");
  UTIL_ASSERT(page_offset <= max_pages() - page_count , "Out of range");
  UTIL_ASSERT(pages_to_bytes(page_offset + page_count) <= size_,
              "Out of range");

  protect_memory(begin_ + pages_to_bytes(page_offset),
                 pages_to_bytes(page_count));
}

void page_allocation::deallocate() const noexcept {
  if (begin_) {
    release_memory(begin_, size_);
  }
}

} // namespace util
