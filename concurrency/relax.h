// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_CONCURRENCY_RELAX_H_
#define DDV_UTILS_CONCURRENCY_RELAX_H_ 1

#if (defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)) &&	\
	!defined(__INTEL_COMPILER) &&										\
	(defined(__x86_64__) || defined(__i386__))

#	define UTILS_RELAX_GNU_

#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))

#	include <immintrin.h>

#	define UTILS_RELAX_MSC_

#endif

namespace util {

inline void thread_relax() noexcept
{
#if defined(UTILS_RELAX_GNU_)

#	undef UTILS_RELAX_GNU_

	__builtin_ia32_pause();

#elif defined(UTILS_RELAX_MSC_)

#	undef UTILS_RELAX_MSC_

	_mm_pause();

#endif
}

} // namespace util

#endif /* DDV_UTILS_CONCURRENCY_RELAX_H_ */
