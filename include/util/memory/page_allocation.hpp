//
// page_allocation.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_MEMORY_PAGE_ALLOCATION_HPP_INCLUDED_
#define DDVAMP_UTIL_MEMORY_PAGE_ALLOCATION_HPP_INCLUDED_ 1

#include <util/memory/view.hpp>

#include <cstddef>
#include <utility>

namespace util {

class [[nodiscard]] page_allocation {
 private:
  ::std::byte *begin_ = nullptr;
  ::std::size_t size_ = 0;

 public:
  ~page_allocation() {
    deallocate();
  }

  page_allocation(page_allocation const &) = delete;
  void operator= (page_allocation const &) = delete;

  page_allocation(page_allocation &&that) noexcept {
    swap(that);
  }

  page_allocation &operator= (page_allocation &&that) noexcept {
    auto(::std::move(that)).swap(*this);
    return *this;
  }

 public:
  page_allocation() noexcept = default;

  [[nodiscard]] ::std::byte *begin() const noexcept {
    return begin_;
  }

  [[nodiscard]] ::std::byte *end() const noexcept {
    return begin_ + size_;
  }

  [[nodiscard]] ::std::size_t size() const noexcept {
    return size_;
  }

  [[nodiscard]] memory_view view() const noexcept {
    return {begin_, size_};
  }

  [[nodiscard]] static ::std::size_t page_size() noexcept;
  [[nodiscard]] static ::std::size_t max_pages() noexcept;

  [[nodiscard]] static ::std::size_t pages_to_bytes(
      ::std::size_t const page_count) noexcept;
  [[nodiscard]] static ::std::size_t bytes_to_pages(
      ::std::size_t const at_least) noexcept;

  // Throws: std::bad_alloc if the storage cannot be obtained or
  //          zero pages are requested
  static page_allocation allocate_pages(::std::size_t count);

  // Precondition: page_count != 0 &&
  //               protected memory in the range [begin_, begin_ + size_)
  void protect_pages(::std::size_t const page_offset,
                     ::std::size_t const page_count);

  void swap(page_allocation &that) noexcept {
    ::std::swap(begin_, that.begin_);
    ::std::swap(size_, that.size_);
  }

 private:
  page_allocation(::std::byte *begin, ::std::size_t size) noexcept
      : begin_(begin), size_(size) {}

  void deallocate() const noexcept;
};

inline void swap(page_allocation &lhs, page_allocation &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace util

#endif /* DDVAMP_UTIL_MEMORY_PAGE_ALLOCATION_HPP_INCLUDED_ */
