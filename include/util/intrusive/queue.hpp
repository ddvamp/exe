//
// queue.hpp
// ~~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_INTRUSIVE_QUEUE_HPP_INCLUDED_
#define DDVAMP_UTIL_INTRUSIVE_QUEUE_HPP_INCLUDED_ 1

#include <util/debug/assert.hpp>

#include <memory>
#include <utility>

namespace util {

template <typename T>
class intrusive_queue {
 private:
  T *head_ = nullptr;
  T *tail_ = nullptr;

 public:
  constexpr ~intrusive_queue() = default;

  intrusive_queue(intrusive_queue const &) = delete;
  void operator= (intrusive_queue const &) = delete;

  constexpr intrusive_queue(intrusive_queue &&that) noexcept
      : head_(::std::exchange(that.head_, nullptr))
      , tail_(::std::exchange(that.tail_, nullptr)) {}

  constexpr intrusive_queue &operator= (intrusive_queue &&that) noexcept {
    head_ = ::std::exchange(that.head_, nullptr);
    tail_ = ::std::exchange(that.tail_, nullptr);
    return *this;
  }

 public:
  constexpr intrusive_queue() noexcept = default;

  [[nodiscard]] constexpr bool empty() const noexcept {
    return !head_;
  }

  constexpr void push(T &elem) noexcept {
    auto const ptr = ::std::addressof(elem);

    ptr->link(nullptr);

    if (empty()) [[unlikely]] {
      head_ = tail_ = ptr;
    } else {
      ::std::exchange(tail_, ptr)->link(ptr);
    }
  }

  [[nodiscard]] constexpr T &pop() noexcept {
    UTIL_ASSERT(!empty(), "Queue is empty");

    auto const ptr = ::std::exchange(head_, head_->next());
    tail_ = (head_ ? tail_ : nullptr);
    return *ptr;
  }

  constexpr void swap(intrusive_queue &that) noexcept {
    ::std::swap(head_, that.head_);
    ::std::swap(tail_, that.tail_);
  }
};

template <typename T>
constexpr void swap(intrusive_queue<T> &lhs, intrusive_queue<T> &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace util

#endif /* DDVAMP_UTIL_INTRUSIVE_QUEUE_HPP_INCLUDED_ */
