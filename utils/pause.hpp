// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_PAUSE_HPP_INCLUDED_
#define DDVAMP_UTILS_PAUSE_HPP_INCLUDED_ 1

#if (defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)) && \
    !defined(__INTEL_COMPILER) &&                                     \
    (defined(__x86_64__) || defined(__i386__))
#	define UTILS_PAUSE_OPTION_GNU_
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
#	include <immintrin.h>
#	define UTILS_PAUSE_OPTION_MSC_
#endif

namespace utils {

inline void pause() noexcept {
#if defined(UTILS_PAUSE_OPTION_GNU_)
#	undef UTILS_PAUSE_OPTION_GNU_
    __builtin_ia32_pause();
#elif defined(UTILS_PAUSE_OPTION_MSC_)
#	undef UTILS_PAUSE_OPTION_MSC_
    _mm_pause();
#else
    // TODO
#endif
}

} // namespace utils

#endif /* DDVAMP_UTILS_PAUSE_HPP_INCLUDED_ */
