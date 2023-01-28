// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_MEMORY_VIEW_H_
#define DDV_UTILS_MEMORY_VIEW_H_ 1

#include <span>

namespace utils {

using memory_view = ::std::span<::std::byte>;

}

#endif /* DDV_UTILS_MEMORY_VIEW_H_ */
