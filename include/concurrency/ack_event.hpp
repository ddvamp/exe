//
// ack_event.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_ACK_EVENT_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_ACK_EVENT_HPP_INCLUDED_ 1

#include <concurrency/pause.hpp>

#include <util/mm/release_sequence.hpp>

#include <atomic>

namespace concurrency {

/**
 *  Synchronization primitive for waiting for a event and
 *  sharing data through it.
 */
class AckEvent {
 private:
  enum class Phase {
    Init,
    Notified,
    Done
  };
  using enum Phase;

  ::std::atomic<Phase> phase_ = Init;

  // To guarantee the expected implementation.
  static_assert(::std::atomic<Phase>::is_always_lock_free);

 public:
  bool IsReady() const noexcept {
    return phase_.load(::std::memory_order_acquire) == Done;
  }

  void Wait() const noexcept {
    phase_.wait(Init, ::std::memory_order_relaxed);
    while (phase_.load(::std::memory_order_relaxed) != Done) {
      Pause();
    }
    ::util::sync_with_release_sequences(phase_);
  }

  void Fire() noexcept {
    phase_.store(Notified, ::std::memory_order_relaxed);
    phase_.notify_all();
    phase_.store(Done, ::std::memory_order_release);
  }

  // In case of instance reuse.
  void Reset() noexcept {
    phase_.store(Init, ::std::memory_order_relaxed);
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_ACK_EVENT_HPP_INCLUDED_ */
