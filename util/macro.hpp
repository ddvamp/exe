// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_MACRO_HPP_INCLUDED_
#define DDVAMP_UTIL_MACRO_HPP_INCLUDED_ 1

// No-op
#ifndef UTIL_NOTHING
# define UTIL_NOTHING static_cast<void>(0)
#else
# error "UTIL_NOTHING macro is already defined somewhere else"
#endif

// Evaluates expr and discards it
#ifndef UTIL_IGNORE
# define UTIL_IGNORE(expr) static_cast<void>(expr)
#else
# error "UTIL_IGNORE macro is already defined somewhere else"
#endif

// MSVC workaround
#ifndef UTIL_NO_UNIQUE_ADDRESS
# if defined(_MSC_VER) && _MSC_VER >= 1929
#   define UTIL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
# else
#   define UTIL_NO_UNIQUE_ADDRESS [[no_unique_address]]
# endif
#else
# error "UTIL_NO_UNIQUE_ADDRESS macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTIL_MACRO_HPP_INCLUDED_ */
