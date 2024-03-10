// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ 1

#include <atomic>
#include <new> // hardware_destructive_interference_size

#include <utils/debug/assert.hpp>
#include <utils/intrusive/forward_list.hpp>
#include <utils/macro.hpp>
#include <utils/pause.hpp>
#include <utils/utility.hpp>

namespace concurrency {

class QueueSpinlock {
private:
    struct Node : ::utils::intrusive_concurrent_forward_list_node<Node> {
        ::std::atomic_bool is_owner_ = false;
    };

    // To guarantee the expected implementation
    static_assert(::std::atomic_bool::is_always_lock_free);
    static_assert(::std::atomic<Node *>::is_always_lock_free);

    alignas (::std::hardware_destructive_interference_size)
        ::std::atomic<Node *> tail_ = nullptr;

public:
    class Guard {
    private:
        QueueSpinlock &lock_;
        Node node_{};

    public:
        ~Guard() {
            lock_.Unlock(node_);
        }

        Guard(Guard const &) = delete;
        void operator= (Guard const &) = delete;

        Guard(Guard &&) = delete;
        void operator= (Guard &&) = delete;

    public:
        explicit Guard(QueueSpinlock &lock) noexcept
            : lock_(lock) {
            lock_.Lock(node_);
        }
    };

public:
    ~QueueSpinlock() {
        UTILS_ASSERT(!tail_.load(::std::memory_order_relaxed),
            "QueueSpinlock is destroyed during use");
    }

    QueueSpinlock(QueueSpinlock const &) = delete;
    void operator= (QueueSpinlock const &) = delete;

    QueueSpinlock(QueueSpinlock &&) = delete;
    void operator= (QueueSpinlock &&) = delete;

public:
    QueueSpinlock() = default;

    [[nodiscard]] Guard MakeGuard() noexcept {
        return Guard(*this);
    }

private:
    void Lock(Node &node) noexcept {
        auto const prev = tail_.exchange(&node, ::std::memory_order_acq_rel);
        if (!prev) [[likely]] {
            return;
        }

        prev->next_.store(&node, ::std::memory_order_relaxed);
        while (!node.is_owner_.load(::std::memory_order_relaxed)) {
            ::utils::pause();
        }

        // Synchronize
        UTILS_IGNORE(node.is_owner_.load(::std::memory_order_acquire));
    }

    void Unlock(Node &node) noexcept {
        auto *next = node.next_.load(::std::memory_order_relaxed);
        if (next) [[likely]] {
            // Synchronize
            UTILS_IGNORE(node.next_.load(::std::memory_order_acquire));
        } else if (tail_.compare_exchange_strong(::utils::temporary(&node),
            nullptr, ::std::memory_order_release,
            ::std::memory_order_acquire)) [[likely]] {
            return;
        } else while (!(next = node.next_.load(::std::memory_order_relaxed))) {
            ::utils::pause();
        }

        next->is_owner_.store(true, ::std::memory_order_release);
    }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ */
