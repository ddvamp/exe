//
// debug.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_DEBUG_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_HPP_INCLUDED_ 1

/**
 *  Debug utilities
 *  To disable, use -DUTIL_DISABLE_DEBUG
 */

#include <util/debug/assert.hpp>      // IWYU pragma: export
#include <util/debug/assume.hpp>      // IWYU pragma: export
#include <util/debug/run.hpp>         // IWYU pragma: export
#include <util/debug/unreachable.hpp> // IWYU pragma: export

#endif /* DDVAMP_UTIL_DEBUG_HPP_INCLUDED_ */
