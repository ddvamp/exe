// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONTEXT_STACK_HPP_INCLUDED_
#define DDVAMP_CONTEXT_STACK_HPP_INCLUDED_ 1

#include <cstddef>
#include <utility>

#include <utils/debug/assert.hpp>
#include <utils/memory/page_allocation.hpp>

namespace context {

class [[nodiscard]] Stack {
 private:
  ::utils::page_allocation allocation_;

 public:
  ~Stack() = default;

  Stack(Stack const &) = delete;
  void operator= (Stack const &) = delete;

  Stack(Stack &&) = default;
  Stack &operator= (Stack &&) = default;

 public:
  Stack() = default;

  static Stack AllocatePages(::std::size_t const count) {
    UTILS_ASSERT(count != 0, "An empty Stack was requested");

    Stack res(::utils::page_allocation::allocate_pages(count + 1));
    res.allocation_.protect_pages(0, 1);
    return res;
  }

  static Stack AllocateBytes(::std::size_t const at_least) {
    return AllocatePages(::utils::page_allocation::bytes_to_pages(at_least));
  }

  [[nodiscard]] ::std::size_t AllocationSize() const noexcept {
    return allocation_.size();
  }

  [[nodiscard]] ::utils::memory_view View() noexcept {
    return allocation_.view();
  }

  static Stack Acquire(::utils::memory_view view) noexcept {
    return ::utils::page_allocation::acquire(view);
  }

  [[nodiscard]] ::utils::memory_view Release() && noexcept {
    return ::std::move(allocation_).release();
  }

 private:
  Stack(::utils::page_allocation &&allocation) noexcept
      : allocation_(::std::move(allocation)) {}
};

}  // namespace context

#endif /* DDVAMP_CONTEXT_STACK_HPP_INCLUDED_ */
