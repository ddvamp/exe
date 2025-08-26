// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "exe/sched/tp/thread_pool.hpp"

#include <util/abort.hpp>
#include <util/debug/assert.hpp>

namespace exe::sched::tp {

namespace {

thread_local ThreadPool *current_pool = nullptr;

::std::size_t NormalizeWorkerCount(::std::size_t const requested) noexcept {
  auto const available = ::std::thread::hardware_concurrency();
  return (available == 0 || requested < available) ? requested : available;
}

} // namespace

/* static */ ThreadPool *ThreadPool::Current() noexcept {
  return current_pool;
}

ThreadPool::~ThreadPool() {
  UTIL_ASSERT(state_ == State::stopped,
              "Thread pool was not stopped before the destruction");
}

void ThreadPool::WorkLoop() noexcept try {
  current_pool = this;

  // TODO: Exception handling (mutex.lock())
  while (auto task = tasks_.Pop()) {
    task->Run();
  }
} catch (...) {
  UTIL_ABORT("Exception inside ThreadPool's worker thread");
}

void ThreadPool::Start() {
  UTIL_ASSERT(state_ == State::created, "Thread pool has already been started");
  state_ = State::started;

  auto cnt = worker_count_;
  workers_.reserve(cnt);
  do {
    workers_.emplace_back(&ThreadPool::WorkLoop, this);
  } while (--cnt != 0);
}

void ThreadPool::JoinWorkerThreads() {
  for (auto &w : workers_) {
    w.join();
  }
}

ThreadPool::ThreadPool(::std::size_t const workers)
    : worker_count_(NormalizeWorkerCount(workers)) {
  UTIL_ASSERT(workers != 0, "Zero-size thread pool was requested");
}

ThreadPool::ThreadPool(::std::size_t const workers, launch_t)
    : ThreadPool(workers) {
  Start();
}

/* virtual */ void ThreadPool::Submit(task::TaskBase *task) {
  UTIL_ASSERT(state_ == State::started, "Using thread pool before start");
  UTIL_VERIFY(tasks_.Push(task), "Using thread pool after stop");
}

void ThreadPool::Stop() {
  UTIL_ASSERT(state_ == State::started,
              "Attempt to stop non-working thread pool");
  state_ = State::stopped;

  tasks_.Close();
  JoinWorkerThreads();
}

} // namespace exe::sched::tp
