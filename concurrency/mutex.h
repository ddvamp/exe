// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_MUTEX_H_
#define DDV_CONCURRENCY_MUTEX_H_ 1

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <mutex>
#include <span>

#include "util/defer.h"
#include "util/type_traits.h"

namespace concurrency {

namespace detail {

struct any_lock_base {
	void *lock_;
	void(*fn_)(void *, bool);

	void lock() const
	{
		fn_(lock_, true);
	}

	void unlock() const noexcept
	{
		fn_(lock_, false);
	}
};

struct any_lock {
	any_lock_base base_;

	operator void const * () const noexcept
	{
		return base_.lock_;
	}

	any_lock_base const *operator-> () const noexcept
	{
		return &base_;
	}
};

template <typename LockPtr>
void lockImpl(::std::span<LockPtr> locks)
{
	::std::ranges::sort(locks);

	auto next = ::std::size_t(0);

	auto unlock_on_exception = ::util::defer(
		[&]() noexcept {
			while (next-- != 0) {
				locks[next]->unlock();
			}
		}
	);

	do {
		locks[next]->lock();
	} while (++next != size(locks));

	next = 0;
}

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename Lock>
concept BasicLockable = requires (Lock &lock) {
	{ lock.lock() };
	{ lock.unlock() } noexcept;
};

template <BasicLockable Lock1, BasicLockable Lock2, BasicLockable ...LockN>
void lock(Lock1 &lock1, Lock2 &lock2, LockN &...lockn)
{
	auto list = []<typename ...Locks>(Locks &...locks) {
		if constexpr (::util::are_all_same_v<Locks...>) {
			return ::std::array{::std::addressof(locks)...};
		} else {
			return ::std::array{
				detail::any_lock{
					.lock_ = ::std::addressof(locks),
					.fn_ = [](void *obj, bool lock) {
						auto m = static_cast<Locks *>(obj);
						if (lock) {
							m->lock();
						} else {
							m->unlock();
						}
					},
				}...
			};
		}
	} (lock1, lock2, lockn...);

	detail::lockImpl(list);
}

template <typename ...MutexTypes>
::std::scoped_lock<MutexTypes...> make_scoped_lock(MutexTypes &...m)
{
	if constexpr (sizeof...(m) < 2) {
		return ::std::scoped_lock(m...);
	} else {
		::util::lock(m...);
		return {::std::adopt_lock, m...};
	}
}

} // namespace concurrency

#endif /* DDV_CONCURRENCY_MUTEX_H_ */
