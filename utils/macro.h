// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_MACRO_H_
#define DDV_UTILS_MACRO_H_ 1

// no-op
#define UTILS_NOTHING static_cast<void>(0)

// evaluates expression and discards it
#define UTILS_IGNORE(expr) static_cast<void>(expr)

#endif /* DDV_UTILS_MACRO_H_ */
