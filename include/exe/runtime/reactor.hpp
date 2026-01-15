//
// reactor.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_REACTOR_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_REACTOR_HPP_INCLUDED_ 1

#include <unistd.h> // close, read, write
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cerrno>
#include <cstdint>
#include <ranges>
#include <system_error>
#include <utility>

[[nodiscard("Pure")]] inline ::std::error_code ErrnoToErrorCode() noexcept {
  // generic for errno, system for other
  return {errno, ::std::generic_category()};
}

[[noreturn]] inline void ThrowErrorCode(::std::error_code ec) {
  throw ::std::system_error(ec);
}

[[noreturn]] inline void ThrowErrno() {
  ThrowErrorCode(ErrnoToErrorCode());
}

namespace exe::runtime {

struct Operation {
 protected:
  ~Operation() = default;

 public:
  virtual void OnEvent(::std::uint32_t events) noexcept = 0;
};

struct DummyOperation : Operation {
  void OnEvent(::std::uint32_t) noexcept override {}
};

class Reactor {
 private:
  int epoll_fd_ = -1;
  int wakeup_fd_ = -1; // eventfd
  DummyOperation on_wakeup_;

  int max_events_ = 0;
  inline static constexpr int kMaxEventsHard = 1024;

  ::std::atomic_bool stop_requested_ = false;
  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);

 public:
  ~Reactor() noexcept = default;

  Reactor(Reactor const &) = delete;
  void operator= (Reactor const &) = delete;

  Reactor(Reactor &&) = delete;
  void operator= (Reactor &&) = delete;

 public:
  Reactor() noexcept = default;

  void Init(int max_events) {
    epoll_fd_ = ::epoll_create1(0);
    if (epoll_fd_ == -1) [[unlikely]] {
      ThrowErrno();
    }

    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK);
    if (wakeup_fd_ == -1) [[unlikely]] {
      auto const ec = ErrnoToErrorCode();
      CloseFd(epoll_fd_);
      ThrowErrorCode(ec);
    }

    auto const ec = AddFd(wakeup_fd_, EPOLLIN | EPOLLET, on_wakeup_);
    if (ec) [[unlikely]] {
      CloseFd(wakeup_fd_);
      CloseFd(epoll_fd_);
      ThrowErrorCode(ec);
    }

    max_events_ = ::std::min(::std::max(max_events, 1), kMaxEventsHard);
  }

  void Stop() noexcept {
    stop_requested_.store(true, ::std::memory_order_relaxed);
    ::std::uint64_t one = 1;
    assert(::write(wakeup_fd_, &one, sizeof(one)) == sizeof(one));
  }

  void PollOnce() noexcept {
    DoPollOnce(max_events_, -1);
  }

  void Run() noexcept {
    while (!stop_requested_.load(::std::memory_order_relaxed)) {
      PollOnce();
    }
  }

  void Reset() noexcept {
    stop_requested_.store(false, ::std::memory_order_relaxed);
    ::std::uint64_t val;
    assert(::read(wakeup_fd_, &val, sizeof(val)) == sizeof(val));
  }

  void Close() noexcept {
    CloseFd(wakeup_fd_);
    CloseFd(epoll_fd_);
  }

  /* non-throwing fd management */

  [[nodiscard]] ::std::error_code AddFd(int fd, ::std::uint32_t events,
                                        Operation &op) noexcept {
    epoll_event ev = {.events = events, .data = {.ptr = &op}};
    return DoEpollCtl(EPOLL_CTL_ADD, fd, &ev);
  }

  [[nodiscard]] ::std::error_code ModFd(int fd, ::std::uint32_t events,
                                        Operation &op) noexcept {
    epoll_event ev = {.events = events, .data = {.ptr = &op}};
    return DoEpollCtl(EPOLL_CTL_MOD, fd, &ev);
  }

  [[nodiscard]] ::std::error_code DelFd(int fd) noexcept {
    return DoEpollCtl(EPOLL_CTL_DEL, fd, nullptr);
  }

  /* throwing fd management */

  void AddFdOrThrow(int fd, ::std::uint32_t events, Operation &op) {
    if (auto const ec = AddFd(fd, events, op)) [[unlikely]] {
      ThrowErrorCode(ec);
    }
  }

  void ModFdOrThrow(int fd, ::std::uint32_t events, Operation &op) {
    if (auto const ec = ModFd(fd, events, op)) [[unlikely]] {
      ThrowErrorCode(ec);
    }
  }

  void DelFdOrThrow(int fd) {
    if (auto const ec = DelFd(fd)) [[unlikely]] {
      ThrowErrorCode(ec);
    }
  }

 private:
  // Pre: max_events <= kMaxEventsHard
  int DoPollOnce(int maxevents, int timeout) noexcept {
    epoll_event events[kMaxEventsHard];
    auto const cnt = ::epoll_wait(epoll_fd_, events, maxevents, timeout);

    if (cnt == -1) [[unlikely]] {
      assert(errno == EINTR);
      return 0;
    }

    for (auto const &[e, data] : events | ::std::views::take(cnt)) {
      static_cast<Operation *>(data.ptr)->OnEvent(e);
    }

    return cnt;
  }

  [[nodiscard]] ::std::error_code DoEpollCtl(int op, int fd, epoll_event *ev)
      noexcept {
    errno = 0;
    ::epoll_ctl(epoll_fd_, op, fd, ev);
    return ErrnoToErrorCode();
  }

  static void CloseFd(int &fd) noexcept {
    /**
     *  On Linux, even in case of an error, the file is closed.
     *  Аny errors other than EBADF occur when trying to flush.
     *  For live file without flush support, this means no errors
     */
    assert(0 == ::close(::std::exchange(fd, -1)));
  }
};

} // namespace exe::runtime

#endif /* DDVAMP_EXE_RUNTIME_REACTOR_HPP_INCLUDED_ */
