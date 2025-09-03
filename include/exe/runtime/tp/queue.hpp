//
// queue.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_TP_QUEUE_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_TP_QUEUE_HPP_INCLUDED_ 1

#include <exe/runtime/task/task.hpp>

#include <concurrency/intrusive/forward_list.hpp>
#include <util/abort.hpp>
#include <util/defer.hpp>
#include <util/debug/assert.hpp>

#include <condition_variable>
#include <cstdint> // std::uint32_t
#include <mutex>

namespace exe::runtime::tp {

class Queue {
 private:
  ::concurrency::IntrusiveQueue<task::TaskBase> queue_;
  ::std::mutex m_;
  ::std::condition_variable has_elements_;
  ::std::uint32_t waiters_count_ = 0;
  bool is_closed_ = false;

 public:
  ~Queue() {
    UTIL_ASSERT(is_closed_, "Queue is destroyed before it is closed");
  }

  Queue(Queue const &) = delete;
  void operator= (Queue const &) = delete;

  Queue(Queue &&) = delete;
  void operator= (Queue &&) = delete;

 public:
  Queue() = default;

  bool Push(task::TaskBase *task) {
    UTIL_ASSERT(task, "nullptr instead of the task");

    ::std::lock_guard lock(m_);

    if (is_closed_) [[unlikely]] {
      return false;
    }

    queue_.Push(task);

    if (waiters_count_ != 0) {
      has_elements_.notify_one();
    }

    return true;
  }

  [[nodiscard]] task::TaskBase *Pop() {
    ::std::unique_lock lock(m_);

    ::util::defer stop_waiting([&cnt = ++waiters_count_] noexcept { --cnt; });

    while (true) {
      if (!queue_.IsEmpty()) {
        return queue_.Pop();
      }

      if (is_closed_) [[unlikely]] {
        return nullptr;
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

} // namespace exe::runtime::tp

#endif /* DDVAMP_EXE_RUNTIME_TP_QUEUE_HPP_INCLUDED_ */
