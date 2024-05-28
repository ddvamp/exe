// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_SLOT_HPP_INCLUDED_
#define DDVAMP_UTIL_SLOT_HPP_INCLUDED_ 1

#include <memory>
#include <type_traits>
#include <utility>

namespace util {

// Storage for an object
template <typename T>
class place {
 protected:
  union { T value_; };

 public:
  constexpr place() noexcept {}
  constexpr ~place() {}

  place(place const &) = delete;
  void operator= (place const &) = delete;

  place(place &&) = delete;
  void operator= (place &&) = delete;

 public:
  [[nodiscard]] constexpr auto *ptr(this auto &&self) noexcept {
    return ::std::addressof(self.value_);
  }

  [[nodiscard]] constexpr auto &&ref(this auto &&self) noexcept {
    return ::std::forward<decltype(self)>(self).value_;
  }

  constexpr void reset() noexcept (::std::is_nothrow_destructible_v<T>) {
    if constexpr (!::std::is_trivially_destructible_v<T>) {
      value_.~T();
    }
  }

  template <typename ...Ts>
  constexpr T &emplace(Ts &&...ts)
      noexcept (::std::is_nothrow_constructible_v<T, Ts...>) {
    return *::std::construct_at(ptr(), ::std::forward<Ts>(ts)...);
  }

  template <typename U, typename ...Ts>
  constexpr T &emplace(::std::initializer_list<U> ilist, Ts &&...ts)
      noexcept (::std::is_nothrow_constructible_v<T,
                ::std::initializer_list<U> &, Ts...>) {
    return *::std::construct_at(ptr(), ilist, ::std::forward<Ts>(ts)...);
  }

  template <typename U = T>
  constexpr place &operator= (U &&u)
      noexcept (::std::is_nothrow_constructible_v<T, U>) {
    ::std::construct_at(ptr(), ::std::forward<U>(u));
    return *this;
  }

  [[nodiscard]] constexpr auto *operator-> (this auto &&self) noexcept {
    return ::std::addressof(self.value_);
  }

  [[nodiscard]] constexpr auto &&operator* (this auto &&self) noexcept {
    return ::std::forward<decltype(self)>(self).value_;
  }
};


// A low-level holder of an object that expects its absence when filled
// and presence when accessed/destroyed
template <typename T>
class slot : public place<T> {
 public:
  constexpr ~slot() noexcept (noexcept(::std::declval<slot &>().reset())) {
    this->reset();
  }

  using place<T>::operator=;
};

}  // namespace util

#endif  /* DDVAMP_UTIL_SLOT_HPP_INCLUDED_ */
