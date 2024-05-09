// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_LOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_LOCK_HPP_INCLUDED_ 1

#include <algorithm>  // std::ranges::sort
#include <array>
#include <memory>  // std::addressof
#include <mutex>  // std::scoped_lock

#include <utils/defer.hpp>
#include <utils/type_traits.hpp>

namespace concurrency {

namespace detail {

template <typename T>
void Lock(void *m) {
  static_cast<T *>(m)->lock();
}

template <typename T>
void Unlock(void *m) noexcept {
  static_cast<T *>(m)->unlock();
}

struct AnyMutex {
  void *m_;
  void (*lock_)(void *);
  void (*unlock_)(void *) noexcept;

  void Lock() const {
    lock_(m_);
  }

  void Unlock() const noexcept {
    unlock_(m_);
  }

  AnyMutex const *operator-> () const noexcept {
    return this;
  }

  operator void * () const noexcept {
    return m_;
  }
};

template <typename It>
void LockImpl(It const begin, It const end) {
  ::std::ranges::sort(begin, end);

  auto it = begin;

  ::utils::defer unlock_on_exception([&]() noexcept {
    if (it == begin) [[likely]] {
      return;
    }
    do {
      (*--it)->unlock();
    } while (it != begin);
  });

  do {
    (*it)->lock();
  } while (++it != end);

  it = begin;
}

template <typename Lock>
inline constexpr bool is_basic_lockable_v = requires (Lock &lock) {
  { lock.lock() };
  { lock.unlock() };  // must be nothrow
};

}  // namespace detail

template <typename ...Locks>
void Lock(Locks &...locks) requires (
    ::utils::is_all_of_v<(sizeof...(Locks) > 1),
    detail::is_basic_lockable_v<Locks>...>) {
  if constexpr (::utils::is_all_same_v<Locks...>) { 
    ::std::array arr{::std::addressof(locks)...};
    detail::LockImpl(arr.begin(), arr.end());
  } else {
    ::std::array arr{detail::AnyMutex{.m_ = const_cast<void *>(
                                          static_cast<void const volatile *>(
                                              ::std::addressof(locks))),
                                      .lock_ = detail::Lock<Locks>,
                                      .unlock_ = detail::Unlock<Locks>}...};
    detail::LockImpl(arr.begin(), arr.end());
  }
}

template <typename ...Mutexes>
::std::scoped_lock<Mutexes...> MakeScopedLock(Mutexes &...ms) {
  if constexpr (sizeof...(Mutexes) > 1) {
    concurrency::Lock(ms...);
    return ::std::scoped_lock{::std::adopt_lock, ms...};
  } else {
    return ::std::scoped_lock{ms...};
  }
}

}  // namespace concurrency

#endif  /* DDVAMP_CONCURRENCY_LOCK_HPP_INCLUDED_ */
