// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_DEBUG_RUN_HPP_INCLUDED_
#define DDVAMP_UTILS_DEBUG_RUN_HPP_INCLUDED_ 1

// To disable, set macro UTILS_DISABLE_DEBUG

#include <utils/macro.hpp>

// Run function during debug
#ifndef UTILS_DEBUG_RUN
# ifndef UTILS_DISABLE_DEBUG
#   define UTILS_DEBUG_RUN(func, ...) func(__VA_ARGS__)
# else
#	  define UTILS_DEBUG_RUN(func, ...) UTILS_NOTHING
# endif
#else
# error "UTILS_DEBUG_RUN macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTILS_DEBUG_RUN_HPP_INCLUDED_ */
