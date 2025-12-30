//
// mpmc_unbounded_blocking_queue.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_HPP_INCLUDED_ 1

#include <util/defer.hpp>
#include <util/debug/assert.hpp>

#include <condition_variable>
#include <cstdint> // std::uint32_t
#include <deque>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>

namespace concurrency {

template <typename T>
requires (::std::is_nothrow_move_constructible_v<T> &&
          ::std::is_nothrow_destructible_v<T>)
class MPMCUnboundedBlockingQueue {
 private:
  ::std::deque<T> buffer_;
  ::std::mutex m_; // Protects the queue
  ::std::condition_variable has_elements_;
  ::std::uint32_t waiters_count_ = 0;
  bool is_closed_ = false;

 public:
  ~MPMCUnboundedBlockingQueue() {
    UTIL_ASSERT(is_closed_, "Queue is destroyed before it is closed");
  }

  MPMCUnboundedBlockingQueue(MPMCUnboundedBlockingQueue const &) = delete;
  void operator= (MPMCUnboundedBlockingQueue const &) = delete;

  MPMCUnboundedBlockingQueue(MPMCUnboundedBlockingQueue &&) = delete;
  void operator= (MPMCUnboundedBlockingQueue &&) = delete;

 public:
  MPMCUnboundedBlockingQueue() = default;

  // Returns false if the queue is closed
  template <typename ...Args>
  bool Push(Args &&...args) requires (::std::is_constructible_v<T, Args...>) {
    ::std::lock_guard lock(m_);

    if (is_closed_) [[unlikely]] {
      return false;
    }

    buffer_.emplace_back(::std::forward<Args>(args)...);

    if (waiters_count_ != 0) {
      has_elements_.notify_one();
    }

    return true;
  }

  [[nodiscard]] ::std::optional<T> Pop() {
    ::std::unique_lock lock(m_);

    ++waiters_count_;
    ::util::defer stop_waiting([this]() noexcept { --waiters_count_; });

    while (true) {
      if (!buffer_.empty()) {
        ::util::defer cleanup([this]() noexcept { buffer_.pop_front(); });
        return ::std::move(buffer_.front());
      }

      if (is_closed_) [[unlikely]] {
        return ::std::nullopt;
      }

      has_elements_.wait(lock);
    }
  }

  void Close() {
    {
      ::std::lock_guard lock(m_);

      UTIL_ASSERT(!is_closed_, "Queue is already closed");
      is_closed_ = true;

      if (waiters_count_ == 0) {
        return;
      }
    }

    has_elements_.notify_all();
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_MPMC_UNBOUNDED_BLOCKING_QUEUE_HPP_INCLUDED_ */
