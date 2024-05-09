// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ 1

#include <atomic>
#include <type_traits>

namespace concurrency {

namespace detail {

struct SelfTag;

}  // namespace detail

template <typename T = detail::SelfTag>
struct IntrusiveForwardListNode {
  using Node = ::std::conditional_t<::std::is_same_v<T, detail::SelfTag>,
                                    IntrusiveForwardListNode, T>;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

  ::std::atomic<Node *> next_ = nullptr;

  void Link(Node *next) noexcept {
    next_.store(next, ::std::memory_order_relaxed);
  }
};

}  // namespace concurrency

#endif  /* DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ */
