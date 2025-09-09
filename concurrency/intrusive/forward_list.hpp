// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ 1

#include <atomic>

namespace concurrency {

namespace detail {

struct default_node_tag;

template <typename T, typename>
struct node_type {
	using type = T;
};

template <typename T>
struct node_type<default_node_tag, T> {
	using type = T;
};

} // namespace detail

template <typename T = detail::default_node_tag>
struct intrusive_concurrent_forward_list_node {
	using Node =
		detail::node_type<T, intrusive_concurrent_forward_list_node<T>>::type;

	::std::atomic<Node *> next_ = nullptr;

	void link(Node *next) noexcept
	{
		next_.store(next, ::std::memory_order_relaxed);
	}
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_INTRUSIVE_FORWARD_LIST_HPP_INCLUDED_ */
