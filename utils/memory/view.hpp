// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_MEMORY_VIEW_HPP_INCLUDED_
#define DDVAMP_UTILS_MEMORY_VIEW_HPP_INCLUDED_ 1

#include <cstddef>
#include <span>

namespace utils {

using memory_view = ::std::span<::std::byte>;

}  // namespace utils

#endif  /* DDVAMP_UTILS_MEMORY_VIEW_HPP_INCLUDED_ */
