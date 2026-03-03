#pragma once

#include <concurrency/intrusive/forward_list.hpp>
#include <util/utility.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>

namespace exe::future::cancel {

struct IHandler {
 protected:
  // Lifetime cannot be controlled via IHandler *
  ~IHandler() = default;

 public:
  virtual void OnCancelRequested() && noexcept = 0;
};

struct HandlerBase : IHandler,
                     ::concurrency::IntrusiveForwardListNode<HandlerBase> {
 protected:
  // Lifetime cannot be controlled via HandlerBase *
  ~HandlerBase() = default;
};

struct DummyHandler final : HandlerBase {
  void OnCancelRequested() && noexcept override {}
};

class CancelSource {
 private:
  DummyHandler dummy_;
  ::std::atomic<HandlerBase *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<HandlerBase *>::is_always_lock_free);

 public:
  [[nodiscard("Pure")]] bool CancelRequested() const noexcept {
    return dummy_.next_.load(::std::memory_order_relaxed) == &dummy_;
  }

  void AddHandler(HandlerBase &h) noexcept {
    if (CancelRequested()) [[unlikely]] {
      ::std::move(h).OnCancelRequested();
      return;
    }

    h.Link(nullptr);

    HandlerBase *expected = nullptr;
    auto handler = tail_.exchange(&h, ::std::memory_order_acq_rel);
    if (handler->next_.compare_exchange_strong(
                           expected, &h,
                           ::std::memory_order_relaxed)) [[likely]] {
      return;
    }

    if (handler != &dummy_) [[likely]] {
      ::util::sync_with_release_sequences(handler->next_);
      handler->Link(&h);
    } else {
      handler = &h;
      Seal();
    }

    NotifyHandlers(handler);
  }

  void RequestCancel() noexcept {
    if (CancelRequested()) [[likely]] {
      return;
    }

    auto const handlers = dummy_.next_.exchange(&dummy_,
                                                ::std::memory_order_relaxed);
    if (::util::all_of(handlers, handlers != &dummy_)) {
      Seal();
      NotifyHandlers(handlers);
    }
  }

 private:
  void Seal() noexcept {
    tail_.exchange(&dummy_, ::std::memory_order_acquire)->Link(&dummy_);
  }

  void NotifyHandlers(HandlerBase *handler) const noexcept {
    do {
      auto next = handler->Next();
      if (!next) [[unlikely]] {
        if (handler->next_.compare_exchange_strong(
                               next, handler,
                               ::std::memory_order_release)) [[likely]] {
          return;
        }
      }

      ::std::move(*handler).OnCancelRequested();
      handler = next;
    } while (handler != &dummy_);
  }
};

} // namespace exe::future::cancel
