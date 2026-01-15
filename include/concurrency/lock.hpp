//
// lock.hpp
// ~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_LOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_LOCK_HPP_INCLUDED_ 1

#include <util/abort.hpp>
#include <util/defer.hpp>
#include <util/type_traits.hpp>
#include <util/utility.hpp>

#include <algorithm>  // std::ranges::sort
#include <array>
#include <memory>     // std::addressof
#include <mutex>      // std::scoped_lock

namespace concurrency {

namespace detail {

template <typename T>
void Lock(void *m) {
  static_cast<T *>(m)->lock();
}

template <typename T>
void Unlock(void *m) {
  static_cast<T *>(m)->unlock();
}

/**
 *  About comparison of void*
 *  https://quuxplusone.github.io/blog/2019/01/20/std-less-nightmare/
 */
struct AnyMutex {
  void *m_;
  void (*lock_)(void *);
  void (*unlock_)(void *);

  void Lock() const {
    lock_(m_);
  }

  void Unlock() const {
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

  ::util::defer unlock_on_exception([&]() noexcept {
    if (it == begin) [[likely]] {
      return;
    }

    try {
      do {
        (*--it)->unlock();
      } while (it != begin);
    } catch (...) {
      UTIL_ABORT("An object of the type with Cpp17BasicLockable requirement, "
                 "threw an exception when calling the unlock method");
    }
  });

  do {
    (*it)->lock();
  } while (++it != end);

  it = begin;
}

template <typename Lock>
inline constexpr bool is_basic_lockable_v = requires (Lock &lock) {
  { lock.lock() };
  { lock.unlock() }; // Must throws nothing
};

} // namespace detail

/**
 *  To capture locks in a consistent order
 *
 *  Each lock must meet the Cpp17BasicLockable requirement and be atomic.
 *  Atomic means that lock itself must respect this order (use Lock function)
 */
template <typename ...Locks>
void Lock(Locks &...locks)
    requires (::util::is_all_of_v<(sizeof...(Locks) > 1),
                                  detail::is_basic_lockable_v<Locks>...>) {
  if constexpr (::util::is_all_same_v<Locks...>) {
    ::std::array arr{::std::addressof(locks)...};
    detail::LockImpl(arr.begin(), arr.end());
  } else {
    ::std::array arr{detail::AnyMutex{.m_ = ::util::voidify(locks),
                                      .lock_ = detail::Lock<Locks>,
                                      .unlock_ = detail::Unlock<Locks>}...};
    detail::LockImpl(arr.begin(), arr.end());
  }
}

template <typename ...Mutexes>
[[nodiscard]] ::std::scoped_lock<Mutexes...> MakeScopedLock(Mutexes &...ms) {
  if constexpr (sizeof...(Mutexes) > 1) {
    concurrency::Lock(ms...);
    return ::std::scoped_lock{::std::adopt_lock, ms...};
  } else {
    return ::std::scoped_lock{ms...};
  }
}

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_LOCK_HPP_INCLUDED_ */
