// to disable, set a macro UTILS_DISABLE_DEBUG

#ifndef DDV_UTILS_DEBUG_RUN_H_
#define DDV_UTILS_DEBUG_RUN_H_ 1

#include "utils/macro.h"

// run function on debug
#ifdef UTILS_DISABLE_DEBUG
#	define UTILS_RUN(function) UTILS_NOTHING
#else
#	define UTILS_RUN(function) function()
#endif

#endif /* DDV_UTILS_DEBUG_RUN_H_ */
