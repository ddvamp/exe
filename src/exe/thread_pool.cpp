//
// thread_pool.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

// [TODO]: What to do with catch(...)?

#include <exe/runtime/tp/thread_pool.hpp>

#include <exe/runtime/task/task.hpp>

#include <util/abort.hpp>
#include <util/debug/assert.hpp>

#include <cstddef>
#include <thread>
#include <utility>

namespace exe::runtime::tp {

namespace {

thread_local ThreadPool *current_pool = nullptr;

::std::size_t NormalizeWorkerCount(::std::size_t const requested) noexcept {
  ::std::size_t const available = ::std::thread::hardware_concurrency();
  return (available == 0 || requested < available) ? requested : available;
}

} // namespace

/* static */ ThreadPool *ThreadPool::Current() noexcept {
  return current_pool;
}

ThreadPool::~ThreadPool() {
  UTIL_ASSERT(state_ == kStopped,
              "Thread pool was not stopped before the destruction");
}

ThreadPool::ThreadPool(::std::size_t const workers)
    : worker_count_(NormalizeWorkerCount(workers)) {
  UTIL_ASSERT(workers != 0, "Zero-size thread pool was requested");
}

ThreadPool::ThreadPool(Launch, ::std::size_t const workers)
    : ThreadPool(workers) {
  Start();
}

void ThreadPool::Start() {
  UTIL_ASSERT(state_ == kCreated, "Thread pool has already been started");

  auto cnt = worker_count_;
  workers_.reserve(cnt);
  try {
    do {
      workers_.emplace_back(&ThreadPool::WorkLoop, this);
    } while (--cnt != 0);
  } catch (...) {
    UTIL_ABORT("Unexpected exception when starting ThreadPool");
  }

  state_ = kStarted;
}

/* virtual */ void ThreadPool::Submit(task::TaskBase *task) {
  UTIL_ASSERT(state_ == kStarted, "Using a non-working thread pool");
  UTIL_ASSERT(task, "nullptr instead of task");
  tasks_.Push(*task);
}

void ThreadPool::Stop() noexcept try {
  UTIL_ASSERT(state_ == kStarted,
              "Attempt to stop non-working thread pool");

  tasks_.Close();
  JoinWorkerThreads();

  state_ = kStopped;
} catch (...) {
  UTIL_ABORT("Unexpected exception when stopping ThreadPool");
}

void ThreadPool::WorkLoop() noexcept try {
  current_pool = this;

  while (auto task = tasks_.Pop()) {
    ::std::move(*task).Run();
  }
} catch (...) {
  UTIL_ABORT("Unexpected exception inside ThreadPool's worker thread");
}

void ThreadPool::JoinWorkerThreads() {
  for (auto &w : workers_) {
    w.join();
  }
}

} // namespace exe::runtime::tp
