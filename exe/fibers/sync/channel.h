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
	void push(T value)
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


template <typename T>
class ChannelState {
private:
	struct ProxyPtr {
		void *object_;
	};

	struct FiberInfo
		: ProxyPtr
		, ::utils::intrusive_concurrent_forward_list_node<FiberInfo> {
		FiberHandle handle_;
	};

	class WaitQueue {
	private:
		FiberInfo *head_ = nullptr;
		FiberInfo *tail_;

	public:
		[[nodiscard]] bool empty() const noexcept
		{
			return !head_;
		}

		void push(FiberInfo *info) noexcept
		{
			if (empty()) {
				head_ = tail_ = info;
			} else {
				::std::exchange(tail_, info).link(info);
			}
		}

		// precondition: empty() == false
		[[nodiscard]] FiberInfo *take() noexcept
		{
			return ::std::exchange(
				head_,
				head_.next_.load(::std::memory_order_relaxed)
			);
		}
	};

	class ChannelAwaiter : public IAwaiter {
	private:
		FiberInfo info_;
		::concurrency::QueueSpinlock::ManualGuard &lock_;
		WaitQueue &queue_;

	public:
		explicit ChannelAwaiter(void *object,
			::concurrency::QueueSpinlock::ManualGuard &lock,
			WaitQueue &queue) noexcept
			: info_{object}
			, lock_(lock)
			, queue_(queue)
		{}

		void awaitSuspend(FiberHandle &&) noexcept override
		{
			// nothing
		}

		[[nodiscard]] FiberHandle awaitSymmetricSuspend(
			FiberHandle &&current) noexcept override
		{
			info_.handle_ = ::std::move(current);
			queue_.push(&info_);
			lock_.unlock();
			return FiberHandle::invalid();
		}
	};

private:
	::concurrency::QueueSpinlock spinlock_; // protects channel state

	::std::atomic_size_t state_/* = ? TODO*/; // size/closed
	WaitQueue recvq_;
	WaitQueue sendq_;

	ChannelBuffer<T> buffer_;

public:
	bool send(T value)
	{
		auto lock = spinlock_.lock_with_manual();

		if (buffer_.full()) {
			ChannelAwaiter awaiter(::std::addressof(value), lock, sendq_);
			self::suspend(awaiter);

			return true;
		}

		if (recvq_.empty()) {
			buffer_.push(::std::move(value));

			lock.unlock();
			return true;
		}

		auto info = recvq_.take();
		static_cast<::std::optional<T> *>(info->object_)
			->emplace(::std::move(value));
		::std::move(info->handle_).schedule();

		lock.unlock();
		return true;
	}

	bool trySend(T value)
	{}

	::std::optional<T> receive()
	{
		::std::optional<T> result;

		auto lock = spinlock_.lock_with_manual();

		if (buffer_.empty()) {
			ChannelAwaiter awaiter(::std::addressof(result), lock, recvq_);
			self::suspend(awaiter);

			return result;
		}

		result.emplace(::std::move(buffer_.front()));
		buffer_.pop();

		if (!sendq_.empty()) {
			auto info = sendq_.take();
			buffer_.push(::std::move(*static_cast<T *>(info->object_)));
			::std::move(info->handle_).schedule();
		}

		lock.unlock();
		return result;
	}

	::std::pair<::std::optional<T>, bool> tryReceive()
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
		: buffer_(capacity)
	{}
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
		// TODO: �������� �� ������� �������� (� �����������)?
		this->~Self();
		deallocate(this);
	}

private:
	explicit ChannelImpl(::std::size_t const capacity) noexcept
		: ChannelState<T>(capacity)
	{}

	// ��������� ���������� ������ ��� ������������ �� count ���������
	static ::std::size_t calculateImplSize(::std::size_t const count)
	{
		// ������� ������ � ������������ ��� ������������� � 1 ���������
		constexpr auto size = sizeof(Self);
		constexpr auto align = alignof(Self);

		// ���. ������ �� ���������
		if (count <= 1) {
			return size;
		}

		// ������������ ��������� ����� ������
		constexpr auto max_size = ::std::size_t{} - 1;

		// �������������� ���������� ���������
		auto const extra_count = count - 1;

		// ������ ������ ��������
		constexpr auto element_size = sizeof(T);

		// ������������� �� ������ ���� ���. ���������
		if constexpr ([[maybe_unused]] auto overflow_is_possible =
			element_size > 1) {
			constexpr auto max_possible = max_size / element_size;
			if (extra_count > max_possible) {
				throw ::std::bad_array_new_length{};
			}
		}

		// ������ ���� �������������� ���������
		auto const extra_size = extra_count * element_size;

		// ������ �� ������ �� ���. �������� � ������������
		// ���������: size + extra_size + align - 1 > max_size
		if (extra_size > max_size - size - (align - 1)) {
			throw ::std::bad_array_new_length{};
		}

		// �������� align - 1 ���������� ��� ����������� ������������,
		// ��������� ������������ ��������� ����� ���� ������, ��� �������������
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

// TODO: �������� �� ���� ��������, ������� ���������� - not array T
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

	void send(T value)
	{
		impl_->send(::std::move(value));
	}

	bool trySend(T value)
	{
		return impl_->trySend(::std::move(value));
	}

	::std::optional<T> receive()
	{
		return impl_->receive();
	}

	::std::pair<::std::optional<T>, bool> tryReceive()
	{
		return impl_->tryReceive();
	}

	void close()
	{
		impl_->close();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_SYNC_CHANNEL_H_ */
