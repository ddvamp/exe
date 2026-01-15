//
// forward_list.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_
#define DDVAMP_UTIL_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ 1

namespace util {

template <typename>
struct intrusive_forward_list_node;

namespace detail {

struct self_tag {};

template <typename T>
struct node_selector {
  using type = T;
};

template <>
struct node_selector<self_tag> {
  using type = intrusive_forward_list_node<self_tag>;
};

} // namespace detail

template <typename T = detail::self_tag>
struct intrusive_forward_list_node {
  using node = detail::node_selector<T>::type;

  node *next_ = nullptr;

  [[nodiscard]] constexpr node *next() const noexcept {
    return next_;
  }

  constexpr void link(node *next) noexcept {
    next_ = next;
  }
};

} // namespace util

#endif /* DDVAMP_UTIL_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ */
