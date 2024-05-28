// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_MEMORY_PAGE_ALLOCATION_HPP_INCLUDED_
#define DDVAMP_UTIL_MEMORY_PAGE_ALLOCATION_HPP_INCLUDED_ 1

#include <cstddef>

#include "view.hpp"

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
    steal(that);
  }
  page_allocation &operator= (page_allocation &&that) noexcept {
    deallocate();
    steal(that);
    return *this;
  }

 public:
  page_allocation() = default;

  [[nodiscard]] ::std::byte *begin() const noexcept {
    return begin_;
  }

  [[nodiscard]] ::std::byte *end() const noexcept {
    return begin_ + size_;
  }

  [[nodiscard]] ::std::size_t size() const noexcept {
    return size_;
  }

  [[nodiscard]] memory_view view() noexcept {
    return {begin_, size_};
  }

  [[nodiscard]] static ::std::size_t page_size() noexcept;
  [[nodiscard]] static ::std::size_t max_pages() noexcept;

  [[nodiscard]] static ::std::size_t pages_to_bytes(
      ::std::size_t const page_count) noexcept;
  [[nodiscard]] static ::std::size_t bytes_to_pages(
      ::std::size_t const at_least) noexcept;

  // Precondition: count != 0 && count <= max_pages()
  static page_allocation allocate_pages(
      ::std::size_t const count);

  // Precondition: page_count != 0 &&
  //               protected memory in the range [begin_, begin_ + size_)
  void protect_pages(::std::size_t const page_offset,
                     ::std::size_t const page_count);

  static page_allocation acquire(memory_view view) noexcept;

  [[nodiscard]] memory_view release() && noexcept {
    auto res = view();
    reset();
    return res;
  }

 private:
  page_allocation(::std::byte *const begin, ::std::size_t const size) noexcept
      : begin_(begin), size_(size) {}

  void reset() noexcept {
    begin_ = nullptr;
    size_ = 0;
  }

  void steal(page_allocation &that) noexcept {
    begin_ = that.begin_;
    size_ = that.size_;
    that.reset();
  }

  void deallocate() const noexcept;
};

}  // namespace util

#endif  /* DDVAMP_UTIL_MEMORY_PAGE_ALLOCATION_HPP_INCLUDED_ */
