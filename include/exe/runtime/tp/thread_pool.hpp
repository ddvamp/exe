//
// thread_pool.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_TP_THREAD_POOL_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_TP_THREAD_POOL_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>
#include <exe/runtime/tp/queue.hpp>

#include <cstddef>
#include <thread>
#include <vector>

namespace exe::runtime::tp {

struct Launch {
  explicit Launch() = default;
};

inline constexpr Launch launch{};


class ThreadPool final : public task::IScheduler {
 private:
  enum class State {
    kCreated,
    kStarted,
    kStopped
  };

  using enum State;

  ::std::vector<::std::thread> workers_;
  ::std::size_t const worker_count_;
  Queue tasks_;
  State state_ = kCreated;

 public:
  ~ThreadPool();

  ThreadPool(ThreadPool const &) = delete;
  void operator= (ThreadPool const &) = delete;

  ThreadPool(ThreadPool &&) = delete;
  void operator= (ThreadPool &&) = delete;

 public:
  explicit ThreadPool(::std::size_t workers);

  // Creates and immediately starts
  ThreadPool(Launch, ::std::size_t workers);

  [[nodiscard]] static ThreadPool *Current() noexcept;

  // Initializes and starts worker threads
  void Start();

  void Submit(task::TaskBase *task) override;

  // Wait for all tasks to complete and join threads
  void Stop() noexcept;

 private:
  void WorkLoop() noexcept;

  void JoinWorkerThreads();
};

} // namespace exe::runtime::tp

#endif /* DDVAMP_EXE_RUNTIME_TP_THREAD_POOL_HPP_INCLUDED_ */
