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

#include <utility>

namespace util {

template <typename T>
class intrusive_queue {
 private:
  T *head_ = nullptr;
  T *tail_;

 public:
  constexpr ~intrusive_queue() = default;

  intrusive_queue(intrusive_queue const &) = delete;
  void operator= (intrusive_queue const &) = delete;

  intrusive_queue(intrusive_queue &&) = delete;
  void operator= (intrusive_queue &&) = delete;

 public:
  constexpr intrusive_queue() noexcept = default;

  [[nodiscard]] constexpr bool empty() const noexcept {
    return !head_;
  }

  constexpr void push(T *elem) noexcept {
		UTIL_ASSERT(elem, "nullptr instead of node");

    elem->link(nullptr);

    if (empty()) {
      head_ = tail_ = elem;
    } else {
      ::std::exchange(tail_, elem)->link(elem);
    }
  }

  [[nodiscard]] constexpr T *pop() noexcept {
		UTIL_ASSERT(!empty(), "Queue is empty");

    return ::std::exchange(head_, head_->next());
  }
};

} // namespace util

#endif /* DDVAMP_UTIL_INTRUSIVE_QUEUE_HPP_INCLUDED_ */
