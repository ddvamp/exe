//
// pause.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_PAUSE_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_PAUSE_HPP_INCLUDED_ 1

#if (defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)) && \
    !defined(__INTEL_COMPILER) &&                                     \
    (defined(__x86_64__) || defined(__i386__))
# define DDVAMP_PAUSE_OPTION_GNU_
#elif defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
# include <immintrin.h>
# define DDVAMP_PAUSE_OPTION_MSC_
#endif

namespace concurrency {

/* Spin loop hint */
inline void Pause() noexcept {
#ifdef DDVAMP_PAUSE_OPTION_GNU_
# undef DDVAMP_PAUSE_OPTION_GNU_
  __builtin_ia32_pause();
#elifdef DDVAMP_PAUSE_OPTION_MSC_
# undef DDVAMP_PAUSE_OPTION_MSC_
  _mm_pause();
#else
  // [TODO]
#endif
}

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_PAUSE_HPP_INCLUDED_ */
