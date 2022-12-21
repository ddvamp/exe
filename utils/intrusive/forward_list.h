#ifndef DDV_UTILS_INTRUSIVE_FORWARD_LIST_H_
#define DDV_UTILS_INTRUSIVE_FORWARD_LIST_H_ 1

#include <atomic>

namespace utils {

namespace detail {

struct default_node_tag;

} // namespace detail

template <typename T = detail::default_node_tag>
struct intrusive_concurrent_forward_list_node {
	using Node = T;

	::std::atomic<Node *> next_ = nullptr;
};

template <>
struct intrusive_concurrent_forward_list_node<detail::default_node_tag> {
	using Node = intrusive_concurrent_forward_list_node;

	::std::atomic<Node *> next_ = nullptr;
};

} // namespace utils

#endif /* DDV_UTILS_INTRUSIVE_FORWARD_LIST_H_ */
