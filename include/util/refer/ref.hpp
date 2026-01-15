//
// ref.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_
#define DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_ 1

#include <util/debug/assert.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace util {

template <typename T>
requires ::std::is_class_v<T>
class [[nodiscard]] ref {
 private:
  T *ptr_ = nullptr;

 public:
  constexpr ~ref() {
    dec_ref();
  }

  ref(ref const &that) noexcept : ptr_(that.ptr_) {
    inc_ref();
  }

  ref(ref &&that) noexcept : ref(that.release()) {}

  ref &operator= (ref that) noexcept {
    swap(that);
    return *this;
  }

 public:
  constexpr ref() noexcept = default;

  constexpr ref(::std::nullptr_t) noexcept {}

  // Constructs a ref without increasing the counter
  constexpr explicit ref(T *ptr) noexcept : ptr_(ptr) {}

  [[nodiscard]] ::std::size_t use_count() const noexcept
      requires (requires {
                  { ptr_->use_count() } noexcept ->
                      ::std::same_as<::std::size_t>;
                }) {
    return ptr_ ? ptr_->use_count() : 0uz;
  }

  void swap(ref &that) noexcept {
    ::std::swap(ptr_, that.ptr_);
  }

  void reset() noexcept {
    ref().swap(*this);
  }

  // Replaces a ref without increasing the counter
  void reset(T *ptr) noexcept {
    UTIL_ASSERT(ptr_ != ptr, "Self reseting");
    ref(ptr).swap(*this);
  }

  [[nodiscard]] T *get() const noexcept {
    return ptr_;
  }

  [[nodiscard]] T &operator* () const noexcept {
    UTIL_ASSERT(ptr_, "nullptr dereference");
    return *ptr_;
  }

  [[nodiscard]] T *operator-> () const noexcept {
    return ptr_;
  }

  [[nodiscard]] T *release() noexcept {
    return ::std::exchange(ptr_, nullptr);
  }

  explicit operator bool() const noexcept {
    return ptr_;
  }

 private:
  void inc_ref() const noexcept
      requires (requires {
                  { ptr_->inc_ref() } noexcept -> ::std::same_as<void>;
                }) {
    if (ptr_) {
      ptr_->inc_ref();
    }
  }

  void dec_ref() const noexcept
      requires (requires {
                  { ptr_->dec_ref() } noexcept -> ::std::same_as<void>;
                }) {
    if (ptr_) {
      ptr_->dec_ref();
    }
  }
};


template <typename T>
[[nodiscard]] bool operator== (ref<T> const &lhs, ref<T> const &rhs) noexcept {
  return ::std::ranges::equal_to{}(lhs.get(), rhs.get());
}

template <typename T>
[[nodiscard]] auto operator<=> (ref<T> const &lhs, ref<T> const &rhs) noexcept {
  return ::std::compare_three_way{}(lhs.get(), rhs.get());
}

template <typename T>
void swap(ref<T> &lhs, ref<T> &rhs) noexcept {
  lhs.swap(rhs);
}

} // namespace util

#endif /* DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_ */
