//
// page_allocation.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

// This file is for internal use and is not intended for direct inclusion

#include <util/debug/assert.hpp>

#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <functional>
#include <new>

namespace util {

namespace {

/* https://lxadm.com/why-are-page-sizes-always-powers-of-2/ */
::std::size_t get_page_size() noexcept {
  return static_cast<::std::size_t>(::sysconf(_SC_PAGE_SIZE));
}

void *allocate_memory(::std::size_t const size) {
  void *const memory = ::mmap(NULL, size, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS,  -1, 0);
  if (::std::not_equal_to<void *>{}(memory, MAP_FAILED)) [[likely]] {
    return memory;
  }

  // ENOMEM:
  // - no memory is available
  // - the process's maximum number of mappings would have been exceeded
  // - the process's RLIMIT_DATA limit would have been exceeded
  // EINVAL:
  // - size is 0
  // - size is too large
  // - size is not aligned on a page boundary
  UTIL_ASSERT(errno == ENOMEM || errno == EINVAL,
              "Unknown mmap() memory allocation error");
  throw ::std::bad_alloc();
}

void protect_memory(void *const address, ::std::size_t const size) {
  auto const ret = ::mprotect(address, size, PROT_NONE);
  if (ret == 0) [[likely]] {
    return;
  }

  // ENOMEM:
  // - internal kernel structures could not be allocated
  // - changing the protection of a memory region would result in
  //   the total number of mappings with distinct attributes exceeding
  //   the allowed maximum
  UTIL_ASSERT(errno == ENOMEM, "Unknown mprotect() memory protect error");
  throw ::std::bad_alloc();
}

void release_memory(void *const address, ::std::size_t const size) noexcept {
  [[maybe_unused]] auto const ret = ::munmap(address, size);
  UTIL_ASSERT(ret == 0, "Unknown munmap() memory release error");
}

} // namespace

} // namespace util
