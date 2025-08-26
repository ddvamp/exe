// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_TP_THREAD_POOL_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_TP_THREAD_POOL_HPP_INCLUDED_ 1

#include "queue.hpp"
#include <exe/sched/task/scheduler.hpp>

#include <cstddef>
#include <thread>
#include <vector>

namespace exe::sched::tp {

struct launch_t {
  explicit launch_t() = default;
};

inline constexpr launch_t launch{};


class ThreadPool final : public task::IScheduler {
 private:
  enum class State {
    created,
    started,
    stopped
  };

  ::std::vector<::std::thread> workers_;
  ::std::size_t const worker_count_;
  Queue tasks_;
  State state_ = State::created;

 public:
  ~ThreadPool();

  ThreadPool(ThreadPool const &) = delete;
  void operator= (ThreadPool const &) = delete;

  ThreadPool(ThreadPool &&) = delete;
  void operator= (ThreadPool &&) = delete;

 public:
  explicit ThreadPool(::std::size_t workers);

  // Creates and immediately starts
  ThreadPool(::std::size_t workers, launch_t);

  [[nodiscard]] static ThreadPool *Current() noexcept;

  void Submit(task::TaskBase *task) override;

  // Initializes and starts worker threads
  void Start();

  // Wait for all tasks to complete and join threads
  void Stop();

 private:
  void WorkLoop() noexcept;

  void JoinWorkerThreads();
};

} // namespace exe::sched::tp

#endif /* DDVAMP_EXE_SCHED_TP_THREAD_POOL_HPP_INCLUDED_ */
