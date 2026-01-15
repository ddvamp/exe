//
// macro.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_MACRO_HPP_INCLUDED_
#define DDVAMP_UTIL_MACRO_HPP_INCLUDED_ 1

/* No-op */
#ifdef UTIL_NOTHING
# error "UTIL_NOTHING macro could not be defined because it is already defined somewhere else"
#else
# define UTIL_NOTHING static_cast<void>(0)
#endif

/* Evaluates expression and discards it */
#ifdef UTIL_IGNORE
# error "UTIL_IGNORE macro could not be defined because it is already defined somewhere else"
#else
# define UTIL_IGNORE(expr) static_cast<void>(expr)
#endif

/**
 *  MSVC workaround
 *  https://devblogs.microsoft.com/cppblog/msvc-cpp20-and-the-std-cpp20-switch/
 */
#ifdef UTIL_NO_UNIQUE_ADDRESS
# error "UTIL_NO_UNIQUE_ADDRESS macro could not be defined because it is already defined somewhere else"
#elif defined(_MSC_VER) && _MSC_VER >= 1929
# define UTIL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
# define UTIL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#endif /* DDVAMP_UTIL_MACRO_HPP_INCLUDED_ */
