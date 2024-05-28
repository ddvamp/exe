// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_ 1

// To disable, set macro UTIL_DISABLE_DEBUG

#include <util/macro.hpp>

// Run function during debug
#ifndef UTIL_DEBUG_RUN
# ifndef UTIL_DISABLE_DEBUG
#   define UTIL_DEBUG_RUN(func, ...) func(__VA_ARGS__)
# else
#	  define UTIL_DEBUG_RUN(func, ...) UTIL_NOTHING
# endif
#else
# error "UTIL_DEBUG_RUN macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_ */
