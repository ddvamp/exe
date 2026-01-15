//
// t_reactor.cpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2025-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/runtime/reactor.hpp>

#include <unistd.h>
#include <sys/timerfd.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>

class Timer : exe::runtime::Operation {
 private:
  int fd_;
  int times_ = 0;

  exe::runtime::Reactor *r_;

 public:
  ~Timer() {
    close(fd_);
  }

 public:
  Timer() {
    fd_ = ::timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd_ == -1) [[unlikely]] {
      ThrowErrno();
    }
  }

  [[nodiscard("Pure")]] int Times() const noexcept {
    return times_;
  }

  void Arm() noexcept {
    ::itimerspec spec = {.it_value = {.tv_nsec = 10'000'000}};
    assert(::timerfd_settime(fd_, 0, &spec, nullptr) == 0);
  }

  void Register(exe::runtime::Reactor &r) {
    r_ = &r;
    r.AddFdOrThrow(fd_, EPOLLIN | EPOLLET, *this);
  }

  // exe::runtime::Operation
  void OnEvent(::std::uint32_t) noexcept override {
    ::std::cout << "Event\n";
    if (++times_ == 100) [[unlikely]] {
      r_->Stop();
    }
    Arm();
  }
};

int TestReactor() {
  exe::runtime::Reactor reactor;
  reactor.Init(128);

  ::std::cout << "Start\n";

  Timer timer;
  timer.Register(reactor);
  timer.Arm();

  reactor.Run();
  reactor.Close();

  ::std::cout << "End " << timer.Times() << ::std::endl;

  return EXIT_SUCCESS;
}

int main() {
  return TestReactor();
}
