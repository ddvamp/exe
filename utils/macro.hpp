// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_MACRO_HPP_INCLUDED_
#define DDVAMP_UTILS_MACRO_HPP_INCLUDED_ 1

// No-op
#ifndef UTILS_NOTHING
#   define UTILS_NOTHING static_cast<void>(0)
#else
#   error "UTILS_NOTHING macro is already defined somewhere else"
#endif

// Evaluates expr and discards it
#ifndef UTILS_IGNORE
#   define UTILS_IGNORE(expr) static_cast<void>(expr)
#else
#   error "UTILS_IGNORE macro is already defined somewhere else"
#endif

// MSVC workaround
#ifndef UTILS_NO_UNIQUE_ADDRESS
#   if defined(_MSC_VER) && _MSC_VER >= 1929
#       define UTILS_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#   else
#       define UTILS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#   endif
#else
#   error "UTILS_NO_UNIQUE_ADDRESS macro is already defined somewhere else"
#endif

#endif /* DDVAMP_UTILS_MACRO_HPP_INCLUDED_ */
