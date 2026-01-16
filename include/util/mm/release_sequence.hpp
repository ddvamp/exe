//
// release_sequence.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_MM_RELEASE_SEQUENCE_HPP_INCLUDED_
#define DDVAMP_UTIL_MM_RELEASE_SEQUENCE_HPP_INCLUDED_ 1

#include <util/macro.hpp>

#include <atomic>

namespace util {

/**
 *  Synchronizes with release sequences on `a`.
 *  The loaded value is intentionally ignored
 *
 *  This is not equivalent to atomic_thread_fence(memory_order_acquire).
 *  A fence does not synchronize with release sequences on a specific atomic
 */
template <typename T>
inline void SyncWithReleaseSequences(::std::atomic<T> const &a) noexcept {
  UTIL_IGNORE(a.load(::std::memory_order_acquire));
}

} // namespace util

#endif /* DDVAMP_UTIL_MM_RELEASE_SEQUENCE_HPP_INCLUDED_ */
