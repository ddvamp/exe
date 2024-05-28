// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_
#define DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_ 1

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include <util/macro.hpp>

namespace util {

template <typename Derived>
class ref_count;


template <typename T>
inline constexpr bool is_ref_counted_v =
    ::std::derived_from<T, ref_count<::std::remove_const_t<T>>> &&
    requires (T const &t) { { t.destroy_self() } noexcept; };

template <typename T>
struct is_ref_counted : ::std::bool_constant<is_ref_counted_v<T>> {};

template <typename T>
concept ref_counted = is_ref_counted_v<T>;


#ifdef UTIL_REF_VALIDATE_TYPE
# error "UTIL_REF_VALIDATE_TYPE macro is already defined somewhere else"
#else
# define UTIL_REF_VALIDATE_TYPE(T)                                         \
      static_assert(ref_counted<T>,                                        \
                    "Class " #T " does not inherit ref_count<" #T "> or "  \
                    "has not defined the function destroy_self()")
#endif


template <typename T>
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

  ref(ref &&that) noexcept : ptr_(that.ptr_) {
    that.ptr_ = nullptr;
  }

  ref &operator= (ref that) noexcept {
    swap(that);
    return *this;
  }

 public:
  constexpr ref() = default;

  constexpr ref(::std::nullptr_t) noexcept {}

  constexpr explicit ref(T *ptr) noexcept : ptr_(ptr) {}

  [[nodiscard]] ::std::size_t use_count() const noexcept {
    return ptr_ ? get_checked()->use_count() : 0;
  }

  void swap(ref &that) noexcept {
    ::std::swap(ptr_, that.ptr_);
  }

  void reset() noexcept {
    ref().swap(*this);
  }

  void reset(T *ptr) noexcept {
    ref(ptr).swap(*this);
  }

  [[nodiscard]] T *get() const noexcept {
    return ptr_;
  }

  [[nodiscard]] T &operator* () const noexcept {
    return *ptr_;
  }

  [[nodiscard]] T *operator-> () const noexcept {
    return get_checked();
  }

  explicit operator bool() const noexcept {
    return ptr_;
  }

 private:
  [[nodiscard]] T *get_checked() const noexcept {
    UTIL_REF_VALIDATE_TYPE(T);
    return ptr_;
  }

  void inc_ref() const noexcept {
    if (ptr_) {
      get_checked()->inc_ref();
    }
  }

  void dec_ref() const noexcept {
    if (ptr_) {
      get_checked()->dec_ref();
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

}  // namespace util

#endif  /* DDVAMP_UTIL_REFER_REF_HPP_INCLUDED_ */
