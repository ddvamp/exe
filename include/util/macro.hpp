//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_MACRO_HPP_INCLUDED_
#define DDVAMP_UTIL_MACRO_HPP_INCLUDED_ 1

// no-op
#define UTIL_NOTHING static_cast<void>(0)

// evaluates expression and discards it
#define UTIL_IGNORE(expr) static_cast<void>(expr)

#if defined(_MSC_VER) && _MSC_VER >= 1929
#	define UTIL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#	define UTIL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#endif /* DDVAMP_UTIL_MACRO_HPP_INCLUDED_ */
