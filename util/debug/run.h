// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

// to disable, set a macro UTILS_DISABLE_DEBUG

#ifndef DDV_UTILS_DEBUG_RUN_H_
#define DDV_UTILS_DEBUG_RUN_H_ 1

#include "util/macro.h"

// run function on debug
#ifdef UTILS_DISABLE_DEBUG
#	define UTILS_RUN(function, ...) UTILS_NOTHING
#else
#	define UTILS_RUN(function, ...) function(__VA_ARGS__)
#endif

#endif /* DDV_UTILS_DEBUG_RUN_H_ */
