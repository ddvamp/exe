// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ 1

#include <atomic>
#include <new> // std::hardware_destructive_interference_size

#include <utils/debug/assert.hpp>
#include <utils/intrusive/forward_list.hpp>
#include <utils/pause.hpp>
#include <utils/utility.hpp>

namespace concurrency {

class QueueSpinlock {
private:
    struct Node : ::utils::intrusive_concurrent_forward_list_node<Node> {
        ::std::atomic_bool locked_ = false;
    };

    // To guarantee the expected implementation
    static_assert(::std::atomic_bool::is_always_lock_free);

    alignas (::std::hardware_destructive_interference_size)
        Node dummy_{};
    alignas (::std::hardware_destructive_interference_size)
        ::std::atomic<Node *> tail_ = &dummy_;

public:
    ~QueueSpinlock() {
        UTILS_ASSERT(!IsLocked(), "QueueSpinlock is destroyed during use");
    }

    QueueSpinlock(QueueSpinlock const &) = delete;
    void operator= (QueueSpinlock const &) = delete;

    QueueSpinlock(QueueSpinlock &&) = delete;
    void operator= (QueueSpinlock &&) = delete;

public:
    QueueSpinlock() = default;

    class Token final {
    private:
        QueueSpinlock &lock_;
        Node node_;

    public:
        ~Token() = default;

        Token(Token const &) = delete;
        void operator= (Token const &) = delete;

        Token(Token &&) = delete;
        void operator= (Token &&) = delete;

    public:
        explicit Token(QueueSpinlock &spinlock) noexcept
            : lock_(spinlock)
            , node_{&spinlock.dummy_} {}

        [[nodiscard]] bool TryLock() noexcept {
            return lock_.TryLock();
        }

        void Lock() noexcept {
            lock_.Lock(node_);
        }

        void Unlock() noexcept {
            lock_.Unlock(node_);
        }

        // Cpp17Lockable
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

    [[nodiscard]] Token GetToken() noexcept {
        return Token(*this);
    }

    class Guard final {
    private:
        Token token_;

    public:
        ~Guard() {
            token_.Unlock();
        }

        Guard(Guard const &) = delete;
        void operator= (Guard const &) = delete;

        Guard(Guard &&) = delete;
        void operator= (Guard &&) = delete;

    public:
        explicit Guard(QueueSpinlock &lock) noexcept
            : token_(lock) {
            token_.Lock();
        }
    };

    [[nodiscard]] Guard MakeGuard() noexcept {
        return Guard(*this);
    }

private:
    [[nodiscard]] bool IsLocked() const noexcept {
        return dummy_.locked_.load(::std::memory_order_relaxed);
    }

    [[nodiscard]] bool TryLock() noexcept {
        if (IsLocked()) [[unlikely]] {
            return false;
        }

        return dummy_.locked_.compare_exchange_weak(
            ::utils::temporary(false), true, ::std::memory_order_acquire,
            ::std::memory_order_relaxed);
    }

    void Lock(Node &node) noexcept {
        if (!TryLock()) [[unlikely]] {
            LockCold(node);
        }
    }

    void LockCold(Node &node) noexcept {
        node.link(nullptr);
        node.locked_.store(true, ::std::memory_order_relaxed);

        auto const pred = tail_.exchange(&node, ::std::memory_order_acq_rel);
        if (pred == &dummy_) [[likely]] {
            while (!TryLock()) {
                ::utils::pause();
            }

            return;
        }

        pred->next_.store(&node, ::std::memory_order_release);

        while (node.locked_.load(::std::memory_order_acquire)) {
            ::utils::pause();
        }
    }

    void Unlock(Node &node) noexcept {
        auto *succ = node.next_.load(::std::memory_order_acquire);
        if (!succ) [[unlikely]] {
            succ = UnlockCold(node);
        }

        succ->locked_.store(false, ::std::memory_order_release);
        node.link(&dummy_);
    }

    [[nodiscard]] Node *UnlockCold(Node &node) noexcept {
        auto succ = &dummy_;

        auto const last = tail_.compare_exchange_strong(
            ::utils::temporary(&node), &dummy_, ::std::memory_order_relaxed,
            ::std::memory_order_acquire);
        if (!last) [[unlikely]] {
            while (!(succ = node.next_.load(::std::memory_order_relaxed))) {
                ::utils::pause();
            }
        }

        return succ;
    }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ */
