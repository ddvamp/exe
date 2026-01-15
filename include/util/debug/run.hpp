//
// run.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_ 1

/* To disable, use -DUTIL_DISABLE_DEBUG */

#include <util/macro.hpp>

/* Run function during debug */
#ifdef UTIL_DEBUG_RUN
# error "UTIL_DEBUG_RUN macro could not be defined because it is already defined somewhere else"
#elifdef UTIL_DISABLE_DEBUG
# define UTIL_DEBUG_RUN(func, ...) UTIL_NOTHING
#else
# define UTIL_DEBUG_RUN(func, ...) func(__VA_ARGS__)
#endif

#endif /* DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_ */
