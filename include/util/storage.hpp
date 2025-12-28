//
// storage.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_STORAGE_HPP_INCLUDED_
#define DDVAMP_UTIL_STORAGE_HPP_INCLUDED_ 1

#include <util/type_traits.hpp>

#include <initializer_list>
#include <memory>
#include <utility>

namespace util {

template <typename T>
concept suitable_for_storage =
    ::std::is_object_v<T> && !::std::is_array_v<T> && !is_qualified_v<T>;

/**
 *  A low-level storage with the proper size and alignment for
 *  an object of the specified type. Does not participate in
 *  the management of the stored object
 */
template <suitable_for_storage T>
class storage {
 protected:
  union { T object_; };

 public:
  constexpr ~storage() {}

  storage(storage const &) = delete;
  void operator= (storage const &) = delete;

  storage(storage &&) = delete;
  void operator= (storage &&) = delete;

 public:
  constexpr storage() noexcept {}

  [[nodiscard("Pure")]] constexpr auto *ptr(this auto &&self) noexcept {
    return ::std::addressof(self.object_);
  }

  [[nodiscard("Pure")]] constexpr auto *operator-> (this auto &&self) noexcept {
    return ::std::addressof(self.object_);
  }

  [[nodiscard("Pure")]] constexpr auto &&ref(this auto &&self) noexcept {
    return ::std::forward<decltype(self)>(self).object_;
  }

  [[nodiscard("Pure")]] constexpr auto &&operator* (this auto &&self) noexcept {
    return ::std::forward<decltype(self)>(self).object_;
  }

  inline static constexpr bool is_reset_needed_v =
      !::std::is_trivially_destructible_v<T>;

  constexpr void reset()
      noexcept (::std::is_nothrow_destructible_v<T>)
      requires (::std::is_destructible_v<T>) {
    if constexpr (is_reset_needed_v) {
      ::std::ranges::destroy_at(ptr());
    }
  }

  template <typename ...Ts>
  constexpr T &emplace(Ts &&...ts)
      noexcept (::std::is_nothrow_constructible_v<T, Ts &&...>)
      requires (::std::is_constructible_v<T, Ts &&...>) {
    return *::std::ranges::construct_at(ptr(), ::std::forward<Ts>(ts)...);
  }

  template <typename U, typename ...Ts>
  constexpr T &emplace(::std::initializer_list<U> ilist, Ts &&...ts)
      noexcept (::std::is_nothrow_constructible_v<
                    T, ::std::initializer_list<U> &, Ts &&...>)
      requires (::std::is_constructible_v<
                    T, ::std::initializer_list<U> &, Ts &&...>) {
    return *::std::ranges::construct_at(ptr(), ilist,
                                        ::std::forward<Ts>(ts)...);
  }

  template <typename U = T>
  constexpr storage &operator= (U &&u)
      noexcept (::std::is_nothrow_constructible_v<T, U &&>)
      requires (::std::is_constructible_v<T, U &&>) {
    ::std::ranges::construct_at(ptr(), ::std::forward<U>(u));
    return *this;
  }
};

template <typename T>
concept suitable_for_slot =
    suitable_for_storage<T> && ::std::is_destructible_v<T>;

/**
 *  A storage that automatically destroys a stored object. If there is no object
 *  at the time of destruction, the behavior is undefined
 */
template <suitable_for_slot T>
class slot : public storage<T> {
 public:
  constexpr ~slot() noexcept (noexcept(this->reset())) {
    this->reset();
  }

  using storage<T>::operator=;
};

} // namespace util

#endif /* DDVAMP_UTIL_STORAGE_HPP_INCLUDED_ */
