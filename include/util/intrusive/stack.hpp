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

  intrusive_stack(intrusive_stack &&) = delete;
  void operator= (intrusive_stack &&) = delete;

 public:
  constexpr intrusive_stack() noexcept = default;

  [[nodiscard]] constexpr bool empty() const noexcept {
    return !top_;
  }

  constexpr void push(T *elem) noexcept {
		UTIL_ASSERT(elem, "nullptr instead of node");

    elem->link(::std::exchange(top_, elem));
  }

  [[nodiscard]] constexpr T *pop() noexcept {
		UTIL_ASSERT(!empty(), "Stack is empty");

    return ::std::exchange(top_, top_->next());
  }
};

} // namespace util

#endif /* DDVAMP_UTIL_INTRUSIVE_STACK_HPP_INCLUDED_ */
