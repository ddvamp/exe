//
// view.hpp
// ~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_MEMORY_VIEW_HPP_INCLUDED_
#define DDVAMP_UTIL_MEMORY_VIEW_HPP_INCLUDED_ 1

#include <cstddef>
#include <span>

namespace util {

using memory_view = ::std::span<::std::byte>;

} // namespace util

#endif /* DDVAMP_UTIL_MEMORY_VIEW_HPP_INCLUDED_ */
