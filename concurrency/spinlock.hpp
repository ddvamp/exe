// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_ 1

#include <atomic>
#include <new>

#include <utils/pause.hpp>

namespace concurrency {

class Spinlock {
private:
    alignas (::std::hardware_destructive_interference_size)
        ::std::atomic_flag locked_;

public:
    [[nodiscard]] bool TryLock() noexcept {
        return !locked_.test_and_set(::std::memory_order_acquire);
    }

    void Lock() noexcept {
        while (!TryLock()) {
            while (locked_.test(::std::memory_order_relaxed)) {
                ::utils::pause();
            }
        }
    }

    void Unlock() noexcept {
        locked_.clear(::std::memory_order_release);
    }

    // Lockable
    // https://eel.is/c++draft/thread.req.lockable.req

    [[nodiscard]] bool try_lock() noexcept {
        return TryLock();
    }

    void lock() noexcept {
        Lock();
    }

    void unlock() noexcept {
        Unlock();
    }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_SPINLOCK_HPP_INCLUDED_ */
