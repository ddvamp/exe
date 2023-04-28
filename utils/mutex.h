// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_MUTEX_H_
#define DDV_UTILS_MUTEX_H_ 1

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <mutex>
#include <tuple>

#include "utils/defer.h"
#include "utils/type_traits.h"

namespace utils {

namespace detail {

template <typename ...Locks>
void lockAny(Locks &...locks)
{
	auto sorted_locks = ::std::array{
		::std::tuple{
			static_cast<void *>(::std::addressof(locks)),
			+[](void *lock) {
				static_cast<Locks *>(lock)->lock();
			},
			+[](void *lock) noexcept {
				static_cast<Locks *>(lock)->unlock();
			},
		}...
	};

	::std::ranges::sort(
		sorted_locks,
		{},
		[](auto const &t) noexcept {
			return ::std::get<0>(t);
		}
	);

	::std::size_t next = 0;

	auto unlock_on_exception = defer(
		[&] noexcept {
			while (next-- != 0) {
				auto [ptr, _, fn] = sorted_locks[next];
				fn(ptr);
			}
		}
	);

	do {
		auto [ptr, fn, _] = sorted_locks[next];
		fn(ptr);
	} while (++next != size(sorted_locks));

	next = 0;
}

template <typename ...Locks>
void lockSame(Locks &...locks)
{
	auto sorted_locks = ::std::array{
		::std::addressof(locks)...
	};

	::std::ranges::sort(sorted_locks);

	::std::size_t next = 0;

	auto unlock_on_exception = defer(
		[&] noexcept {
			while (next-- != 0) {
				sorted_locks[next]->unlock();
			}
		}
	);

	do {
		sorted_locks[next]->lock();
	} while (++next != size(sorted_locks));

	next = 0;
}

} // namespace detail

template <typename Lock>
concept BasicLockable = requires (Lock &lock) {
	{ lock.lock() };
	{ lock.unlock() } noexcept;
};

template <BasicLockable Lock1, BasicLockable Lock2, BasicLockable ...LockN>
void lock(Lock1 &lock1, Lock2 &lock2, LockN &...lockn)
{
	if constexpr (are_all_same_v<Lock1, Lock2, LockN...>) {
		detail::lockSame(lock1, lock2, lockn...);
	} else {
		detail::lockAny(lock1, lock2, lockn...);
	}
}

template <typename ...MutexTypes>
::std::scoped_lock<MutexTypes...> make_scoped_lock(MutexTypes &...m)
{
	if constexpr (sizeof...(m) < 2) {
		return ::std::scoped_lock(m...);
	} else {
		utils::lock(m...);
		return {::std::adopt_lock, m...};
	}
}

} // namespace utils

#endif /* DDV_UTILS_MUTEX_H_ */
