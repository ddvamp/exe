// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_MACRO_H_
#define DDV_UTILS_MACRO_H_ 1

// no-op
#define UTILS_NOTHING static_cast<void>(0)

// evaluates expression and discards it
#define UTILS_IGNORE(expr) static_cast<void>(expr)

#if defined(_MSC_VER) && _MSC_VER >= 1929
#	define UTILS_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#	define UTILS_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#endif /* DDV_UTILS_MACRO_H_ */
