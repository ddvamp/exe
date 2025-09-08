//
// dummy_test.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/fiber/api.hpp>

#include <exe/fiber/sync/barrier.hpp>
#include <exe/fiber/sync/condvar.hpp>
#include <exe/fiber/sync/event.hpp>
#include <exe/fiber/sync/mutex.hpp>
#include <exe/fiber/sync/strand.hpp>
#include <exe/fiber/sync/wait_group.hpp>

#include <exe/runtime/inline.hpp>
#include <exe/runtime/safe_scheduler.hpp>
#include <exe/runtime/thread_pool.hpp>

#include <atomic>
#include <iostream>

#define REPEAT(repetitions) for (auto cnt = repetitions; cnt != 0; --cnt)

int main() {
  exe::runtime::ThreadPool pool(6);
  exe::runtime::SafeScheduler safe_pool(pool);

  exe::fiber::Strand strand;

  std::atomic<int> a = 0;
  std::atomic<bool> f = false;

  auto const kFiberCount = 10000;
  auto const kRepeatCount = 1500;

  auto const fiber_body = [&]() noexcept {
    REPEAT(kRepeatCount) {
      strand.Combine([]{});
      exe::fiber::self::Yield();
    }

    if (++a == kFiberCount) {
      f.store(true);
      f.notify_all();
    }
  };

  pool.Start();

  REPEAT(kFiberCount) {
    exe::fiber::Go(safe_pool, fiber_body);
  }

  f.wait(false);

  pool.Stop();

  std::cout << "Done!\n";

  return 0;
}
