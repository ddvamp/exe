//
// stack.hpp
// ~~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_INTRUSIVE_STACK_HPP_INCLUDED_
#define DDVAMP_UTIL_INTRUSIVE_STACK_HPP_INCLUDED_ 1

#include <util/debug/assert.hpp>

#include <memory>
#include <utility>

namespace util {

template <typename T>
class intrusive_stack {
 private:
  T *top_ = nullptr;

 public:
  constexpr ~intrusive_stack() = default;

  intrusive_stack(intrusive_stack const &) = delete;
  void operator= (intrusive_stack const &) = delete;

  constexpr intrusive_stack(intrusive_stack &&that) noexcept
      : top_(::std::exchange(that.top_, nullptr)) {}

  constexpr intrusive_stack &operator= (intrusive_stack &&that) noexcept {
    top_ = ::std::exchange(that.top_, nullptr);
    return *this;
  }

 public:
  constexpr intrusive_stack() noexcept = default;

  [[nodiscard]] constexpr bool empty() const noexcept {
    return !top_;
  }

  constexpr void push(T &elem) noexcept {
    auto const ptr = ::std::addressof(elem);

    ptr->link(::std::exchange(top_, ptr));
  }

  [[nodiscard]] constexpr T &pop() noexcept {
    UTIL_ASSERT(!empty(), "Stack is empty");

    return *::std::exchange(top_, top_->next());
  }

  constexpr void swap(intrusive_stack &that) noexcept {
    ::std::swap(top_, that.top_);
  }
};

template <typename T>
constexpr void swap(intrusive_stack<T> &lhs, intrusive_stack<T> &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace util

#endif /* DDVAMP_UTIL_INTRUSIVE_STACK_HPP_INCLUDED_ */
