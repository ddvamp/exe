//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

// to disable, set a macro UTIL_DISABLE_DEBUG

#ifndef DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_ 1

#include "util/macro.hpp"

// run function on debug
#ifdef UTIL_DISABLE_DEBUG
#	define UTIL_RUN(function, ...) UTIL_NOTHING
#else
#	define UTIL_RUN(function, ...) function(__VA_ARGS__)
#endif

#endif /* DDVAMP_UTIL_DEBUG_RUN_HPP_INCLUDED_ */
