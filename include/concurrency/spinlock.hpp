//
// spinlock.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_ 1

#include <concurrency/pause.hpp>

#include <util/debug/assert.hpp>

#include <atomic>
#include <new> // std::hardware_destructive_interference_size

namespace concurrency {

class alignas (::std::hardware_destructive_interference_size) Spinlock {
 private:
  ::std::atomic_flag locked_;

 public:
  ~Spinlock() {
    UTIL_ASSERT(!IsLocked(), "Spinlock is destroyed when locked");
  }

  Spinlock(Spinlock const &) = delete;
  void operator= (Spinlock const &) = delete;

  Spinlock(Spinlock &&) = delete;
  void operator= (Spinlock &&) = delete;

 public:
  Spinlock() = default;

  // This method is not intended to be used in a loop. Use Lock instead
  [[nodiscard]] bool TryLock() noexcept {
    return !locked_.test_and_set(::std::memory_order_acquire);
  }

  void Lock() noexcept {
    while (!TryLock()) {
      while (IsLocked()) {
        Pause(); // [TODO]: Better spin wait
      }
    }
  }

  void Unlock() noexcept {
    locked_.clear(::std::memory_order_release);
  }

  // Cpp17Lockable
  // https://eel.is/c++draft/thread.req.lockable.req

  // This method is not intended to be used in a loop. Use lock instead
  [[nodiscard]] bool try_lock() noexcept {
    return TryLock();
  }

  void lock() noexcept {
    Lock();
  }

  void unlock() noexcept {
    Unlock();
  }

 private:
  [[nodiscard]] bool IsLocked() const noexcept {
    return locked_.test(::std::memory_order_relaxed);
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_ */
