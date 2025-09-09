// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_SYNC_CHANNEL_H_
#define DDV_EXE_FIBER_SYNC_CHANNEL_H_ 1

#include <atomic>
#include <bit>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>

#include "concurrency/queue_spinlock.h"

#include "exe/fiber/api.h"
#include "exe/fiber/core/awaiter.h"
#include "exe/fiber/core/handle.h"

#include "util/debug.h"
#include "util/refer/ref_counted_ptr.h"
#include "util/utility.h"

namespace exe::fiber {

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename ...>
class Selector;


template <typename T>
class ChannelBuffer {
private:
	::std::size_t recv_ = 0;
	::std::size_t send_ = 0;
	::std::size_t const capacity_;

	::std::size_t const index_mask_ = calculateStorageSize(capacity_) - 1;
	union {
		T storage_;
	};

public:
	[[nodiscard]] static auto calculateStorageSize(
		::std::size_t const capacity) noexcept
	{
		return ::std::bit_ceil(capacity);
	}

	~ChannelBuffer() noexcept
	{
		if constexpr (!::std::is_trivially_destructible_v<T>) {
			while (!empty()) {
				pop();
			}
		}
	}

	explicit ChannelBuffer(::std::size_t const capacity) noexcept
		: capacity_(capacity)
	{}

	[[nodiscard]] ::std::size_t capacity() const noexcept
	{
		return capacity_;
	}

	[[nodiscard]] ::std::size_t size() const noexcept
	{
		return send_ - recv_;
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return size() == 0;
	}

	[[nodiscard]] bool full() const noexcept
	{
		return size() == capacity();
	}

	[[nodiscard]] T &front() noexcept
	{
		return getPtr()[getRecvIdx()];
	}

	void push(T &&value) noexcept
	{
		::std::ranges::construct_at(
			getPtr() + getSendIdx(),
			::std::move(value)
		);

		++send_;
	}

	void pop() noexcept
	{
		if constexpr (!::std::is_trivially_destructible_v<T>) {
			::std::ranges::destroy_at(getPtr() + getRecvIdx());
		}

		++recv_;
	}

private:
	auto getPtr() noexcept
	{
		return ::std::addressof(storage_);
	}

	auto getRecvIdx() const noexcept
	{
		return recv_ & index_mask_;
	}

	auto getSendIdx() const noexcept
	{
		return send_ & index_mask_;
	}
};


class ChannelWaitQueue {
public:
	struct Node {
		void *object_;
		FiberHandle handle_;
		Node *next_ = nullptr;
		bool success_;

		void schedule(bool success) noexcept
		{
			success_ = success;
			::std::move(handle_).schedule();
		}

		template <typename T>
		T &getObj() const noexcept
		{
			return *static_cast<T *>(object_);
		}
	};

private:
	::std::atomic<Node *> head_ = nullptr;
	Node *tail_;

public:
	[[nodiscard]] bool empty(::std::memory_order order) const noexcept
	{
		return !head_.load(order);
	}

	void push(Node *node) noexcept
	{
		constexpr auto order = ::std::memory_order_relaxed;

		if (empty(order)) {
			head_.store(tail_ = node, order);
		} else {
			::std::exchange(tail_, node)->next_ = node;
		}
	}

	// precondition: empty() == false
	[[nodiscard]] Node *take() noexcept
	{
		constexpr auto order = ::std::memory_order_relaxed;

		auto const result = head_.load(order);
		head_.store(result->next_, order);
		return result;
	}

	[[nodiscard]] Node *takeAll() noexcept
	{
		constexpr auto order = ::std::memory_order_relaxed;

		auto const result = head_.load(order);
		head_.store(nullptr, order);
		return result;
	}
};


class ChannelAwaiter : public IAwaiter {
private:
	ChannelWaitQueue::Node fiber_info_;
	::concurrency::QueueSpinlock::LockToken &lock_;

public:
	template <typename T>
	ChannelAwaiter(T &object, ::concurrency::QueueSpinlock::LockToken &lock,
		ChannelWaitQueue &queue) noexcept
		: fiber_info_{::std::addressof(object)}
		, lock_(lock)
	{
		queue.push(&fiber_info_);
	}

	void awaitSuspend(FiberHandle &&) noexcept override
	{
		// nothing
	}

	[[nodiscard]] FiberHandle awaitSymmetricSuspend(
		FiberHandle &&current) noexcept override
	{
		fiber_info_.handle_ = ::std::move(current);
		lock_.unlock();
		return FiberHandle::invalid();
	}

	[[nodiscard]] bool success() const noexcept
	{
		return fiber_info_.success_;
	}
};


template <typename T>
class ChannelState {
private:
	using OptT = ::std::optional<T>;

	::concurrency::QueueSpinlock spinlock_; // protects channel state

	::std::atomic_size_t size_ = 0;
	::std::atomic_bool closed_ = false;

	ChannelWaitQueue recvq_;
	ChannelWaitQueue sendq_;

	ChannelBuffer<T> buffer_;

public:
	void send(T &&value) noexcept
	{
		sendImpl(value, /* nonblocking */ false);
	}

	// TODO: how not to lose value when false?
	OptT trySend(T &&value) noexcept
	{
		return
			(!closed_.load(::std::memory_order_relaxed) && full()) ||
			!sendImpl(value, /* nonblocking */ true)
			? OptT(::std::move(value))
			: OptT(::std::nullopt);
	}

	OptT receive() noexcept
	{
		OptT result;

		receiveImpl(result, /* nonblocking */ false);

		return result;
	}

	::std::pair<OptT, bool> tryReceive() noexcept
	{
		::std::pair<OptT, bool> result;

		if (empty()) {
			if (!closed_.load(::std::memory_order_seq_cst)) [[likely]] {
				return result;
			}

			if (empty()) {
				result.second = true;
				return result;
			}
		}

		result.second = receiveImpl(result.first, /* nonblocking */ true);

		return result;
	}

	void close() noexcept
	{
		constexpr auto order = ::std::memory_order_relaxed;

		auto token = spinlock_.get_token();

		UTIL_ASSERT(!closed_.load(order), "close of closed channel");

		closed_.store(true, order);

		// fast assertion
		UTIL_ASSERT(sendq_.empty(order), "send on closed channel");

		auto *receiver = recvq_.takeAll();

		token.unlock();

		while (receiver) {
			// load next_ before schedule
			::std::exchange(receiver, receiver->next_)
				->schedule(/* success */ false);
		}
	}

protected:
	static auto calculateElementsCount(::std::size_t const capacity) noexcept
	{
		return ChannelBuffer<T>::calculateStorageSize(capacity);
	}

	explicit ChannelState(::std::size_t const capacity) noexcept
		: buffer_(capacity)
	{}

private:
	::std::size_t capacity() const noexcept
	{
		return buffer_.capacity();
	}

	bool unbuffered() const noexcept
	{
		return capacity() == 0;
	}

	// TODO: is there any better memory orderings?
	bool empty() const noexcept
	{
		auto const order = ::std::memory_order_seq_cst;

		if (unbuffered()) [[unlikely]] {
			return sendq_.empty(order);
		}

		return size_.load(order) == 0;
	}

	bool full() const noexcept
	{
		auto const order = ::std::memory_order_relaxed;

		if (unbuffered()) [[unlikely]] {
			return recvq_.empty(order);
		}

		return size_.load(order) == capacity();
	}

	bool sendImpl(T &value, bool const nonblocking) noexcept
	{
		constexpr auto order = ::std::memory_order_relaxed;

		auto token = spinlock_.get_token();

		// TODO: how to handle go panic?
		auto const is_open = !closed_.load(order);
		UTIL_ASSERT(is_open, "send on closed channel");

		auto const has_receivers = !recvq_.empty(order);
		if (has_receivers) {
			auto * const receiver = recvq_.take();

			token.unlock();

			auto &object = receiver->getObj<OptT>();
			object.emplace(::std::move(value));

			receiver->schedule(/* success */ true);

			return true;
		}

		auto const has_slots = !buffer_.full();
		if (has_slots) {
			buffer_.push(::std::move(value));

			auto const new_size = size_.load(order) + 1;
			size_.store(new_size, order);

			token.unlock();
			return true;
		}

		if (nonblocking) [[unlikely]] {
			token.unlock();
			return false;
		}

		auto awaiter = ChannelAwaiter(value, token, sendq_);
		self::suspend(awaiter);

		// check if it was woken up when closing
		UTIL_ASSERT(
			awaiter.success(),
			"send on closed channel"
		);

		return true;
	}

	bool receiveImpl(OptT &opt, bool const nonblocking) noexcept
	{
		constexpr auto order = ::std::memory_order_relaxed;

		auto token = spinlock_.get_token();

		auto const is_closed = closed_.load(order);
		auto const has_elements = !buffer_.empty();
		auto const has_senders = !sendq_.empty(order);

		if (has_senders) {
			auto * const sender = sendq_.take();
			auto &object = sender->getObj<T>();

			if (has_elements) [[likely]] {
				opt.emplace(::std::move(buffer_.front()));
				buffer_.pop();
				buffer_.push(::std::move(object));
			} else {
				opt.emplace(::std::move(object));
			}

			token.unlock();

			sender->schedule(/* success */ true);

			return is_closed;
		}

		if (has_elements) {
			opt.emplace(::std::move(buffer_.front()));
			buffer_.pop();

			auto const new_size = size_.load(order) - 1;
			size_.store(new_size, order);

			token.unlock();
			return is_closed;
		}

		if (::util::any_of(is_closed, nonblocking)) [[unlikely]] {
			token.unlock();
			return is_closed;
		}

		auto awaiter = ChannelAwaiter(opt, token, recvq_);
		self::suspend(awaiter);

		return !awaiter.success();
	}
};


template <::std::destructible T>
	requires (::std::is_nothrow_move_constructible_v<T>)
class ChannelImpl final
	: public ::util::RefCounted<ChannelImpl<T>>
	, public ChannelState<T> {
private:
	using Self = ChannelImpl;

public:
	[[nodiscard]] static Self *create(::std::size_t const capacity)
	{
		auto const count = Self::calculateElementsCount(capacity);
		auto const raw = allocate(count);
		auto const pimpl = ::new (static_cast<void *>(raw)) Self(capacity);
		return pimpl;
	}

	void destroySelf() const noexcept
	{
		// TODO: last owner -> queues are empty (check?)
		// should the channel be closed?
		this->~Self();
		deallocate(this);
	}

private:
	explicit ChannelImpl(::std::size_t const capacity) noexcept
		: ChannelState<T>(capacity)
	{}

	static ::std::size_t calculateImplSize(::std::size_t const count)
	{
		constexpr auto size = sizeof(Self);
		constexpr auto align = alignof(Self);

		if (count <= 1) {
			return size;
		}

		constexpr auto max_size = ::std::size_t{} - 1;

		auto const extra_count = count - 1;

		constexpr auto element_size = sizeof(T);

		if constexpr ([[maybe_unused]] auto overflow_is_possible =
			element_size > 1) {
			constexpr auto max_possible = max_size / element_size;
			if (extra_count > max_possible) {
				throw ::std::bad_array_new_length{};
			}
		}

		auto const extra_size = extra_count * element_size;

		if (extra_size > max_size - size - (align - 1)) {
			throw ::std::bad_array_new_length{};
		}

		return size + extra_size + (align - 1) & ~(align - 1);
	}

	static Self *allocate(::std::size_t const count)
	{
		auto const size = calculateImplSize(count);

		constexpr auto align = alignof(Self);
		if constexpr (align > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
			return static_cast<Self *>(
				::operator new(size, ::std::align_val_t{align})
			);
		} else {
			return static_cast<Self *>(::operator new(size));
		}
	}

	static void deallocate(Self const * const ptr) noexcept
	{
		auto const vptr = static_cast<void *>(ptr);

		constexpr auto align = alignof(Self);
		if constexpr (align > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
			::operator delete(vptr, ::std::align_val_t{align});
		} else {
			::operator delete(vptr);
		}
	}
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

// TODO: better specialization for void types
template <typename T>
class Channel {
	template <typename ...>
	friend class detail::Selector;

	using element_type = ::std::remove_cv_t<T>;

	using Impl = detail::ChannelImpl<element_type>;

private:
	::util::RefCountedPtr<Impl> impl_;

public:
	explicit Channel(::std::size_t const capacity)
		: impl_(Impl::create(capacity))
	{}

	// precondition: channel is not closed
	void send(element_type value) noexcept
	{
		impl_->send(::std::move(value));
	}

	// precondition: channel is not closed
	// returns object on failure
	[[nodiscard]] auto trySend(element_type value) noexcept
	{
		return impl_->trySend(::std::move(value));
	}

	// returns object on success
	[[nodiscard]] auto receive() noexcept
	{
		return impl_->receive();
	}

	// first == true (contains object) on success, second == true on close
	[[nodiscard]] auto tryReceive() noexcept
	{
		return impl_->tryReceive();
	}

	// precondition: there is no senders
	void close() noexcept
	{
		impl_->close();
	}
};

template <typename T>
	requires (::std::is_void_v<T>)
class Channel<T> {
	template <typename ...>
	friend class detail::Selector;

	using Impl = detail::ChannelImpl<::util::unit_t>;

private:
	::util::RefCountedPtr<Impl> impl_;

public:
	explicit Channel(::std::size_t const capacity)
		: impl_(Impl::create(capacity))
	{}

	// precondition: channel is not closed
	void send() noexcept
	{
		impl_->send(::util::unit_t{});
	}

	// precondition: channel is not closed
	// returns true on failure (consistent with primary template)
	[[nodiscard]] bool trySend() noexcept
	{
		return impl_->trySend(::util::unit_t{});
	}

	// returns true on success
	[[nodiscard]] bool receive() noexcept
	{
		return impl_->receive();
	}

	// first == true on success, second == true on close
	[[nodiscard]] ::std::pair<bool, bool> tryReceive() noexcept
	{
		auto const [fst, snd] = impl_->tryReceive();
		return {fst, snd};
	}

	// precondition: there is no senders
	void close() noexcept
	{
		impl_->close();
	}
};

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_SYNC_CHANNEL_H_ */
