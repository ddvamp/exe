//
// list.hpp
// ~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_INTRUSIVE_LIST_HPP_INCLUDED_
#define DDVAMP_UTIL_INTRUSIVE_LIST_HPP_INCLUDED_ 1

namespace util {

struct intrusive_list_node {
  intrusive_list_node *next_ = nullptr;
  intrusive_list_node *prev_ = nullptr;

  [[nodiscard]] constexpr bool linked() const noexcept {
    return next_;
  }

  constexpr void link(intrusive_list_node &node) noexcept {
    next_ = &node;
    prev_ = node.prev_;
    prev_->next_ = next_->prev_ = this;
  }

  constexpr void unlink() noexcept {
    prev_->next_ = next_;
    next_->prev_ = prev_;
    prev_ = next_ = nullptr;
  }
};

class intrusive_list {
 private:
  intrusive_list_node dummy_{&dummy_, &dummy_};

 public:
  constexpr ~intrusive_list() = default;

  intrusive_list(intrusive_list const &) = delete;
  void operator= (intrusive_list const &) = delete;

  intrusive_list(intrusive_list &&) = delete;
  void operator= (intrusive_list &&) = delete;

 public:
  constexpr intrusive_list() = default;

  [[nodiscard]] constexpr bool empty() const noexcept {
    return dummy_.next_ == &dummy_;
  }

  constexpr void push_front(intrusive_list_node &node) noexcept {
    node.link(*dummy_.next_);
  }

  constexpr void push_back(intrusive_list_node &node) noexcept {
    node.link(dummy_);
  }

  [[nodiscard]] constexpr intrusive_list_node &pop_front() noexcept {
    auto &node = *dummy_.next_;
    node.unlink();
    return node;
  }

  [[nodiscard]] constexpr intrusive_list_node &pop_back() noexcept {
    auto &node = *dummy_.prev_;
    node.unlink();
    return node;
  }
};

} // namespace util

#endif /* DDVAMP_UTIL_INTRUSIVE_LIST_HPP_INCLUDED_ */
