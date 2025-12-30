//
// forward_list.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ 1

#include <util/debug/assert.hpp>

#include <atomic>

namespace concurrency {

template <typename>
struct IntrusiveForwardListNode;

namespace detail {

struct SelfTag {};

template <typename T>
struct NodeSelector {
  using type = T;
};

template <>
struct NodeSelector<SelfTag> {
  using type = IntrusiveForwardListNode<SelfTag>;
};

} // namespace detail

template <typename T = detail::SelfTag>
struct IntrusiveForwardListNode {
  using Node = detail::NodeSelector<T>::type;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

  ::std::atomic<Node *> next_ = nullptr;

  [[nodiscard]] Node *Next() const noexcept {
    return next_.load(::std::memory_order_relaxed);
  }

  void Link(Node *next) noexcept {
    next_.store(next, ::std::memory_order_relaxed);
  }

  /* util/intrusive support */

  [[nodiscard]] Node* next() const noexcept {
    return Next();
  }

  void link(Node *next) noexcept {
    Link(next);
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ */
