//
// t_channel.cpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2025-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/fiber/api.hpp>
#include <exe/fiber/sync/channel.hpp>
#include <exe/fiber/sync/select.hpp>
#include <exe/runtime/thread_pool.hpp>
#include <exe/runtime/safe_scheduler.hpp>

#include <concurrency/wait_group.hpp>
#include <util/debug/assert.hpp>

#include <cstdlib>
#include <format>
#include <iostream>
#include <thread>

int TestChannel() {
  exe::runtime::ThreadPool pool(3);
  exe::runtime::SafeScheduler sched(pool);
  concurrency::WaitGroup wg;

  constexpr auto kStep = 10;

  pool.Start();

  for (auto step = 0; step != kStep; ++step) {
    exe::fiber::Channel<int> ch1(16);
    exe::fiber::Channel<double> ch2(16);
    exe::fiber::Channel<void *> ch3(16);
    exe::fiber::Channel<int *> ch4(16);

    auto const send_body = [=, &wg]() mutable noexcept {
      int i = 0;
      double d = 0;
      void *vp = nullptr;
      int *ip = nullptr;

      while (true) {
        auto result = exe::fiber::TrySelect(ch1.SendClause(i),
                                            ch2.SendClause(d),
                                            ch3.SendClause(vp),
                                            ch4.SendClause(ip));
        if (result.index() == 0) {
          wg.Done();
          return;
        }
      }
    };

    auto const receive_body = [=, &wg]() mutable noexcept {
      while (true) {
        auto result = exe::fiber::TrySelect(ch1.ReceiveClause(),
                                            ch2.ReceiveClause(),
                                            ch3.ReceiveClause(),
                                            ch4.ReceiveClause());
        if (result.index() == 0) {
          wg.Done();
          return;
        }
      }
    };

    constexpr auto kTask = 256;

    wg.Reset(2 * kTask);

    for (auto cnt = kTask; cnt != 0; --cnt) {
      exe::fiber::Go(sched, send_body);
      exe::fiber::Go(sched, receive_body);
    }

    // [TODO]: Use fiber after timers implementation
    using namespace ::std::chrono_literals;
    ::std::this_thread::sleep_for(100ms);

    ch1.Close();
    ch2.Close();
    ch3.Close();
    ch4.Close();

    wg.Wait();

    ::std::cout << ::std::format("step: {}\n", step + 1);
  }

  pool.Stop();

  return EXIT_SUCCESS;
}

int main() {
  return TestChannel();
}
