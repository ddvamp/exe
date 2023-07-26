// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBERS_SYNC_CHANNEL_H_
#define DDV_EXE_FIBERS_SYNC_CHANNEL_H_ 1

#include <atomic>
#include <bit>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "concurrency/queue_spinlock.h"

#include "exe/fibers/api.h"
#include "exe/fibers/core/awaiter.h"
#include "exe/fibers/core/handle.h"

#include "utils/debug.h"
#include "utils/defer.h"
#include "utils/intrusive/forward_list.h"

namespace utils {

template <::std::destructible Derived>
class RefCounter {
private:
	mutable ::std::atomic_size_t ref_cnt_ = 1;

public:
	::std::size_t useCount() const noexcept
	{
		return ref_cnt_.load(::std::memory_order_relaxed);
	}

	void incRef() const noexcept
	{
		ref_cnt_.fetch_add(1, ::std::memory_order_relaxed);
	}

	void decRef() const noexcept
	{
		if (1 == ref_cnt_.fetch_sub(1, ::std::memory_order_acq_rel)) {
			destroySelf();
		}
	}

private:
	auto self() const noexcept
	{
		static_assert(::std::derived_from<Derived, RefCounter>);

		return static_cast<Derived const *>(this);
	}

	void destroySelf() const noexcept
	{
		self()->destroySelf();
	}
};

////////////////////////////////////////////////////////////////////////////////

// TODO: add check for destroySelf
template <typename T>
inline constexpr bool is_ref_counted_v = ::std::derived_from<
	::std::remove_cv_t<T>,
	RefCounter<::std::remove_cv_t<T>>
>;

template <typename T>
struct is_ref_counted : ::std::bool_constant<is_ref_counted_v<T>> {};


namespace concepts {

template <typename T>
concept RefCounted = is_ref_counted_v<T>;

} // namespace concepts

////////////////////////////////////////////////////////////////////////////////

template <concepts::RefCounted T>
class RefHolder {
private:
	T *ptr_ = nullptr;

public:
	~RefHolder()
	{
		decRef();
	}

	RefHolder(RefHolder const &that) noexcept
		: ptr_(that.ptr_)
	{
		incRef();
	}

	RefHolder(RefHolder &&that) noexcept
	{
		that.swap(*this);
	}

	RefHolder &operator= (RefHolder that) noexcept
	{
		that.swap(*this);
		return *this;
	}

public:
	constexpr RefHolder() noexcept = default;

	constexpr RefHolder(::std::nullptr_t) noexcept
	{}

	explicit RefHolder(T *ptr) noexcept
		: ptr_(ptr)
	{}

	[[nodiscard]] ::std::size_t useCount() const noexcept
	{
		return ptr_ ? ptr_->useCount() : 0;
	}

	void swap(RefHolder &that) noexcept
	{
		::std::swap(ptr_, that.ptr_);
	}

	void reset() noexcept
	{
		RefHolder().swap(*this);
	}

	void reset(T *ptr) noexcept
	{
		RefHolder(ptr).swap(*this);
	}

	[[nodiscard]] T *get() const noexcept
	{
		return ptr_;
	}

	[[nodiscard]] T &operator* () const noexcept
	{
		return *get();
	}

	[[nodiscard]] T *operator-> () const noexcept
	{
		return get();
	}

	explicit operator bool() const noexcept
	{
		return ptr_;
	}

private:
	void incRef() const noexcept
	{
		if (ptr_) {
			ptr_->incRef();
		}
	}

	void decRef() const noexcept
	{
		if (ptr_) {
			ptr_->decRef();
		}
	}
};


template <typename T>
[[nodiscard]] bool operator== (RefHolder<T> const &lhs,
	RefHolder<T> const &rhs) noexcept
{
	return ::std::ranges::equal_to{}(lhs.get(), rhs.get());
}

template <typename T>
[[nodiscard]] auto operator<=> (RefHolder<T> const &lhs,
	RefHolder<T> const &rhs) noexcept
{
	return ::std::compare_three_way{}(lhs.get(), rhs.get());
}


template <typename T>
void swap(RefHolder<T> &lhs, RefHolder<T> &rhs) noexcept
{
	lhs.swap(rhs);
}

} // namespace utils


////////////////////////////////////////////////////////////////////////////////


namespace exe::fibers {

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename ...>
class Selector;


// TODO: void types, check for T const
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
	static auto calculateStorageSize(::std::size_t const capacity) noexcept
	{
		return ::std::bit_ceil(capacity);
	}

	~ChannelBuffer() noexcept
	{
		if constexpr (!::std::is_trivially_destructible_v<T>) {
			// TODO: better algorithm?
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

	// TODO: exceptions, by ref, emplace
	void push(T &&value)
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
	struct NodeImpl {
		void *object_;
		FiberHandle handle_;
	};

	struct Node
		: NodeImpl
		, ::utils::intrusive_concurrent_forward_list_node<Node> {};

private:
	Node *head_ = nullptr;
	Node *tail_;

public:
	[[nodiscard]] bool empty() const noexcept
	{
		return !head_;
	}

	void push(Node *node) noexcept
	{
		if (empty()) {
			head_ = tail_ = node;
		} else {
			::std::exchange(tail_, node)->link(node);
		}
	}

	// precondition: empty() == false
	[[nodiscard]] Node *take() noexcept
	{
		return ::std::exchange(
			head_,
			head_->next_.load(::std::memory_order_relaxed)
		);
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
};


template <typename T>
class ChannelState {
private:
	using OptT = ::std::optional<T>;

	::concurrency::QueueSpinlock spinlock_; // protects channel state

	::std::atomic_size_t state_; // size + 1 bit closed
	ChannelWaitQueue recvq_;
	ChannelWaitQueue sendq_;

	ChannelBuffer<T> buffer_;

public:
	void send(T &&value)
	{
		sendImpl(value, /* nonblocking */ false);
	}

	// TODO: how not to lose value when false?
	OptT trySend(T &&value)
	{
		return
			isOpenAndFull() || !sendImpl(value, /* nonblocking */ true)
			? OptT(::std::move(value))
			: OptT(::std::nullopt);
	}

	OptT receive()
	{
		OptT result;

		auto token = spinlock_.get_token();

		if (buffer_.empty()) {
			auto awaiter = ChannelAwaiter(result, token, recvq_);
			self::suspend(awaiter);

			return result;
		}

		auto unlocker = ::utils::defer([&]() noexcept { token.unlock(); });

		result.emplace(::std::move(buffer_.front()));
		buffer_.pop();

		if (!sendq_.empty()) {
			auto info = sendq_.take();
			buffer_.push(::std::move(*static_cast<T *>(info->object_)));
			::std::move(*info).handle_.schedule();
		}

		return result;
	}

	::std::pair<OptT, bool> tryReceive()
	{}

	void close()
	{

	}

protected:
	static auto calculateElementsCount(::std::size_t const capacity) noexcept
	{
		return ChannelBuffer<T>::calculateStorageSize(capacity);
	}

	explicit ChannelState(::std::size_t const capacity) noexcept
		: state_(capacity << 1)
		, buffer_(capacity)
	{}

private:
	::std::size_t getState() const noexcept
	{
		return state_.load(::std::memory_order_relaxed);
	}

	bool isOpenAndFull() const noexcept
	{
		return getState() == 0;
	}

	static ::std::pair<bool, bool> isOpenAndFullApart(
		::std::size_t const state) noexcept
	{
		constexpr auto mask = ::std::size_t{1};

		return {
			(state & mask) == 0,
			(state & ~mask) == 0
		};
	}

	bool sendImpl(T &value, bool nonblocking)
	{
		auto token = spinlock_.get_token();

		auto const state = getState();
		auto const [open, full] = isOpenAndFullApart(state);

		// TODO: how to handle go panic?
		UTILS_ASSERT(open, "send on closed channel");

		if (full) {
			if (nonblocking) {
				return false;
			}

			auto awaiter = ChannelAwaiter(value, token, sendq_);
			self::suspend(awaiter);

			// check if it was woken up when closing
			[[maybe_unused]] auto const got_object =
				[this]() noexcept {
					auto const state = getState();
					auto const [open, _] = isOpenAndFullApart(state);

					return open;
				};
			UTILS_ASSERT(got_object(), "send on closed channel");

			return true;
		}

		auto unlocker = ::utils::defer([&]() noexcept { token.unlock(); });

		if (recvq_.empty()) {
			buffer_.push(::std::move(value));
			state_.store(state - 2, ::std::memory_order_relaxed);
			return true;
		}

		auto info = recvq_.take();
		static_cast<OptT *>(info->object_)
			->emplace(::std::move(value));
		::std::move(*info).handle_.schedule();
		return true;
	}
};


template <typename T>
class ChannelImpl final
	: public ::utils::RefCounter<ChannelImpl<T>>
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
		// TODO: проверка на пустоту очередей (в деструкторе)?
		this->~Self();
		deallocate(this);
	}

private:
	explicit ChannelImpl(::std::size_t const capacity) noexcept
		: ChannelState<T>(capacity)
	{}

	// высчитать количество памяти под имплеентацию на count элементов
	static ::std::size_t calculateImplSize(::std::size_t const count)
	{
		// базовые размер и выравнивание для имплементации с 1 элементом
		constexpr auto size = sizeof(Self);
		constexpr auto align = alignof(Self);

		// доп. память не требуется
		if (count <= 1) {
			return size;
		}

		// максимальный доступный объём памяти
		constexpr auto max_size = ::std::size_t{} - 1;

		// дополнительное количество элементов
		auto const extra_count = count - 1;

		// размер одного элемента
		constexpr auto element_size = sizeof(T);

		// переполняется ли размер всех доп. элементов
		if constexpr ([[maybe_unused]] auto overflow_is_possible =
			element_size > 1) {
			constexpr auto max_possible = max_size / element_size;
			if (extra_count > max_possible) {
				throw ::std::bad_array_new_length{};
			}
		}

		// размер всех дополнительных элементов
		auto const extra_size = extra_count * element_size;

		// хватит ли памяти на доп. элементы и выравнивание
		// логически: size + extra_size + align - 1 > max_size
		if (extra_size > max_size - size - (align - 1)) {
			throw ::std::bad_array_new_length{};
		}

		// поправка align - 1 необходима для правильного выравнивания,
		// поскольку выравнивание элементов может быть меньше, чем имплементации
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

// TODO: подумать на счёт массивов, текущая реализация - not array T
//		 могут ли бросать конструкторы
template <::std::destructible T>
class Channel {
	template <typename ...>
	friend class detail::Selector;

	using Impl = detail::ChannelImpl<T>;

private:
	::utils::RefHolder<Impl> impl_;

public:
	explicit Channel(::std::size_t const capacity)
		: impl_(Impl::create(capacity))
	{}

	// TODO: exceptions
	auto send(T value)
	{
		return impl_->send(::std::move(value));
	}

	// TODO: exceptions
	auto trySend(T value)
	{
		return impl_->trySend(::std::move(value));
	}

	// TODO: exceptions
	auto receive()
	{
		return impl_->receive();
	}

	// TODO: exceptions
	auto tryReceive()
	{
		return impl_->tryReceive();
	}

	// TODO: exceptions
	auto close()
	{
		return impl_->close();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_CHANNEL_H_ */
