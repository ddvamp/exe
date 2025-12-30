//
// channel.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_CHANNEL_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_CHANNEL_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

#include <concurrency/qspinlock.hpp>
#include <util/storage.hpp>
#include <util/type_traits.hpp>
#include <util/utility.hpp>
#include <util/intrusive/list.hpp>
#include <util/refer/ref.hpp>
#include <util/refer/ref_count.hpp>

#include <bit>
#include <cstddef>
#include <mutex>
#include <new>
#include <optional>

namespace exe::fiber {

namespace detail {

/* [TODO]: ?Move to util */

template <::std::size_t Align>
[[nodiscard]] inline void *Allocate(::std::size_t bytes) {
  if constexpr (Align > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
    return ::operator new(bytes, ::std::align_val_t(Align));
  } else {
    return ::operator new(bytes);
  }
}

template <::std::size_t Align>
inline void Deallocate(void *ptr) noexcept {
  if constexpr (Align > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
    ::operator delete(ptr, ::std::align_val_t(Align));
  } else {
    ::operator delete(ptr);
  }
}

template <typename T>
class ChannelBuffer {
 private:
  using Storage = ::util::storage<T>;

  Storage *const buffer_; // [TODO]: start_lifetime_as_array
  ::std::size_t const index_mask_;
  ::std::size_t send_ = 0;
  ::std::size_t recv_ = 0;

  static_assert(sizeof(Storage) == sizeof(T));
  static_assert(alignof(Storage) == alignof(T));

 public:
  ~ChannelBuffer() noexcept {
    if constexpr (Storage::is_reset_needed_v) {
      while (!IsEmpty()) {
        Pop();
      }
    }
  }

  ChannelBuffer(ChannelBuffer const &) = delete;
  void operator= (ChannelBuffer const &) = delete;

  ChannelBuffer(ChannelBuffer &&) = delete;
  void operator= (ChannelBuffer &&) = delete;

 public:
  ChannelBuffer(void *storage, ::std::size_t capacity) noexcept
      : buffer_(::new (storage) Storage[capacity])
      , index_mask_(capacity - 1) {}

  void Push(T &value) noexcept {
    buffer_[GetSendIdx()] = ::std::move(value);
    ++send_;
  }

  void Pop() noexcept {
    if constexpr (Storage::is_reset_needed_v) {
      buffer_[GetRecvIdx()].reset();
    }
    ++recv_;
  }

  [[nodiscard("Pure")]] T &Front() noexcept {
    return buffer_[GetRecvIdx()].ref();
  }

  [[nodiscard("Pure")]] bool IsEmpty() const noexcept {
    return send_ == recv_;
  }

  [[nodiscard("Pure")]] bool IsFull() const noexcept {
    return send_ - recv_ == index_mask_ + 1;
  }

 private:
  [[nodiscard("Pure")]] ::std::size_t GetSendIdx() const noexcept {
    return send_ & index_mask_;
  }

  [[nodiscard("Pure")]] ::std::size_t GetRecvIdx() const noexcept {
    return recv_ & index_mask_;
  }
};

struct ChannelWaiter : ::util::intrusive_list_node {
 protected:
  ~ChannelWaiter() = default;

 public:
  virtual void Cancel() noexcept = 0;
};

class ChannelAwaiter : public IAwaiter {
 protected:
  FiberHandle handle_;

  using Lock = ::std::unique_lock<::concurrency::QSpinlock::Token>;

 private:
  Lock &lock_;

 public:
  explicit ChannelAwaiter(Lock &lock) noexcept : lock_(lock) {}

  FiberHandle AwaitSymmetricSuspend(FiberHandle &&current) noexcept override {
    handle_ = ::std::move(current);
    lock_.unlock();
    return FiberHandle::Invalid();
  }
};

enum class ChannelOpStatus {
  kClose,
  kWait,
  kReady,
};

template <typename T>
class ChannelState {
 public:
  struct Sender : ChannelWaiter {
   protected:
    ~Sender() = default;

   public:
    [[nodiscard]] virtual ::std::optional<T> TrySend() noexcept = 0;
  };

  struct Receiver : ChannelWaiter {
   protected:
    ~Receiver() = default;

   public:
    [[nodiscard]] virtual bool TryReceive(T &) noexcept = 0;
  };

 private:
  class SendAwaiter final : public Sender, public ChannelAwaiter {
   private:
    T &value_;
    bool is_sent_;

   public:
    SendAwaiter(T &value, Lock &lock) noexcept
        : ChannelAwaiter(lock)
        , value_(value) {}

    [[nodiscard("Pure")]] bool IsSent() const noexcept {
      return is_sent_;
    }

    [[nodiscard]] ::std::optional<T> TrySend() noexcept override {
      ::std::optional<T> result(::std::move(value_));
      Schedule(true);
      return result;
    }

    void Cancel() noexcept override {
      Schedule(false);
    }

   private:
    void Schedule(bool is_sent) noexcept {
      is_sent_ = is_sent;
      ::std::move(handle_).Schedule();
    }
  };

  class ReceiveAwaiter final : public Receiver, public ChannelAwaiter {
   private:
    ::std::optional<T> &value_;

   public:
    ReceiveAwaiter(::std::optional<T> &value, Lock &lock) noexcept
        : ChannelAwaiter(lock)
        , value_(value) {}

    [[nodiscard]] bool TryReceive(T &value) noexcept override {
      value_.emplace(::std::move(value));
      Schedule();
      return true;
    }

    void Cancel() noexcept override {
      Schedule();
    }

   private:
    void Schedule() noexcept {
      ::std::move(handle_).Schedule();
    }
  };

 private:
  ::concurrency::QSpinlock lock_; // Protects state
  ChannelBuffer<T> buffer_;
  bool is_closed_ = false;
  ::util::intrusive_list waitq_;

 protected:
   ChannelState(void *storage, ::std::size_t buffer_size) noexcept
      : buffer_(storage, buffer_size) {}

 public:
  bool Send(T &value, bool nonblock) noexcept {
    auto token = lock_.GetToken();
    ::std::unique_lock lock(token);

    if (is_closed_) [[unlikely]] {
      return false;
    }

    if (buffer_.IsFull()) [[unlikely]] {
      if (nonblock) [[unlikely]] {
        return false;
      }

      SendAwaiter awaiter(value, lock);
      waitq_.push_back(awaiter);
      self::Suspend(awaiter); // token.Unlock()
      return awaiter.IsSent();
    }

    BufferPush(value);

    return true;
  }

  [[nodiscard]] ::std::optional<T> Receive(bool nonblock) noexcept {
    ::std::optional<T> result;

    auto token = lock_.GetToken();
    ::std::unique_lock lock(token);

    if (is_closed_) [[unlikely]] {
      return result;
    }

    if (buffer_.IsEmpty()) [[unlikely]] {
      if (nonblock) [[unlikely]] {
        return result;
      }

      ReceiveAwaiter awaiter(result, lock);
      waitq_.push_back(awaiter);
      self::Suspend(awaiter); // token.Unlock()
      return result;
    }

    result.emplace(::std::move(buffer_.Front()));
    BufferPop();

    return result;
  }

  void Close() noexcept {
    auto guard = Lock();

    is_closed_ = true;

    while (!waitq_.empty()) {
      Pop<ChannelWaiter>().Cancel();
    }
  }

  // For Select

  [[nodiscard]] ChannelOpStatus Send(Sender &sender) noexcept {
    auto guard = Lock();

    if (is_closed_) [[unlikely]] {
      sender.Cancel();
      return ChannelOpStatus::kClose;
    }

    if (buffer_.IsFull()) [[unlikely]] {
      waitq_.push_back(sender);
      return ChannelOpStatus::kWait;
    }

    if (auto value_opt = sender.TrySend()) [[likely]] {
      BufferPush(*value_opt);
    }

    return ChannelOpStatus::kReady;
  }

  [[nodiscard]] ChannelOpStatus Receive(Receiver &receiver) noexcept {
    auto guard = Lock();

    if (is_closed_) [[unlikely]] {
      receiver.Cancel();
      return ChannelOpStatus::kClose;
    }

    if (buffer_.IsEmpty()) [[unlikely]] {
      waitq_.push_back(receiver);
      return ChannelOpStatus::kWait;
    }

    if (receiver.TryReceive(buffer_.Front())) [[likely]] {
      BufferPop();
    }

    return ChannelOpStatus::kReady;
  }

  ::concurrency::QSpinlock::Guard Lock() noexcept {
    return lock_.MakeGuard();
  }

 private:
  template <typename U>
  [[nodiscard]] U &Pop() noexcept {
    return static_cast<U &>(waitq_.pop_front());
  }

  void BufferPush(T &value) noexcept {
    while (!waitq_.empty()) {
      if (Pop<Receiver>().TryReceive(value)) [[likely]] {
        return;
      }
    }

    buffer_.Push(value);
  }

  void BufferPop() noexcept {
    buffer_.Pop();

    while (!waitq_.empty()) {
      if (auto value_opt = Pop<Sender>().TrySend()) [[likely]] {
        buffer_.Push(*value_opt);
        return;
      }
    }
  }
};

template <typename T>
concept SuitableForChannel =
    ::std::is_object_v<T> && !::util::is_qualified_v<T> &&
    ::std::is_nothrow_destructible_v<T> &&
    ::std::is_nothrow_move_constructible_v<T>; // implies !std::is_array_v<T>

template <SuitableForChannel T>
class ChannelImpl final : public ChannelState<T>,
                          public ::util::ref_count<ChannelImpl<T>> {
 private:
  using Impl = ChannelImpl;

  using ChannelState<T>::ChannelState;

 public:
  /**
   *  The channel state and the buffer for the elements are placed in
   *  the single allocation side by side. To do this, it needs to determine
   *  the correct size and alignment
   */
  [[nodiscard]] static Impl *Create(::std::size_t requested) {
    constexpr auto align = ::util::max_alignment_of_v<Impl, T>;
    constexpr ::std::size_t max_size_t = -1;
    // sizeof(Impl) + align - 1 must be <= max_size_t
    static_assert(align <= max_size_t - sizeof(Impl) + 1,
                  "Too strict alignment of T");

    constexpr auto impl_bytes = (sizeof(Impl) + align - 1) / align * align;
    auto const size = ::std::bit_ceil(requested);
    // impl_bytes + size * sizeof(T) must be <= max_size_t
    if (size > (max_size_t - impl_bytes) / sizeof(T)) [[unlikely]] {
      throw ::std::bad_array_new_length();
    }

    auto const buffer_bytes = size * sizeof(T);
    auto const bytes = impl_bytes + buffer_bytes;

    auto const storage = Allocate<align>(bytes);
    auto const impl_ptr = ::new (storage) ::std::byte[bytes];
    auto const buffer_ptr = impl_ptr + impl_bytes;
    return ::new (impl_ptr) Impl(buffer_ptr, size);
  }

  // ref_count
  void destroy_self() const noexcept {
    constexpr auto align = ::util::max_alignment_of_v<Impl, T>;
    this->~Impl();
    Deallocate<align>(::util::voidify(this));
  }
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename T>
struct [[nodiscard]] SendClause {
  using ResultType = ::util::unit_t;
  using ValueType = T;

  ChannelState<T> &chan;
  T &value;
};

template <typename T>
struct [[nodiscard]] ReceiveClause {
  using ResultType = T;
  using ValueType = T;

  ChannelState<T> &chan;
};

template <typename>
inline constexpr bool is_select_clause_v = false;

template <typename T>
inline constexpr bool is_select_clause_v<SendClause<T>> = true;

template <typename T>
inline constexpr bool is_select_clause_v<ReceiveClause<T>> = true;

} // namespace detail

template <typename T>
concept SelectClause = detail::is_select_clause_v<T>;

template <typename T>
class Channel {
 private:
  using Impl = detail::ChannelImpl<T>;

  ::util::ref<Impl> impl_;

 public:
  explicit Channel(::std::size_t size) : impl_(Impl::Create(size)) {}

  bool TrySend(T &value) noexcept {
    return impl_->Send(value, true);
  }

  [[nodiscard]] ::std::optional<T> TryReceive() noexcept {
    return impl_->Receive(true);
  }

  bool Send(T &value) noexcept {
    return impl_->Send(value, false);
  }

  [[nodiscard]] ::std::optional<T> Receive() noexcept {
    return impl_->Receive(false);
  }

  void Close() noexcept {
    impl_->Close();
  }

  // For Select

  detail::SendClause<T> SendClause(T &value) noexcept {
    return {*impl_, value};
  }

  detail::ReceiveClause<T> ReceiveClause() noexcept {
    return {*impl_};
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_CHANNEL_HPP_INCLUDED_ */
