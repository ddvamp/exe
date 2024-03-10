// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_
#define DDVAMP_UTILS_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ 1

#include <atomic>
#include <type_traits>

namespace utils {

namespace detail {

struct default_node_tag;

} // namespace detail

template <typename T = detail::default_node_tag>
struct intrusive_concurrent_forward_list_node {
    using node = ::std::conditional_t<
        ::std::is_same_v<T, detail::default_node_tag>,
        intrusive_concurrent_forward_list_node, T>;

    // To guarantee the expected implementation
    static_assert(::std::atomic<node *>::is_always_lock_free);

    ::std::atomic<node *> next_ = nullptr;

    void link(node *next) noexcept {
        next_.store(next, ::std::memory_order_relaxed);
    }
};

} // namespace utils

#endif /* DDVAMP_UTILS_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ */
