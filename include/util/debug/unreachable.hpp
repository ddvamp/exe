//
// unreachable.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_DEBUG_UNREACHABLE_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_UNREACHABLE_HPP_INCLUDED_ 1

/* To disable, use -DUTIL_DISABLE_DEBUG */

#include <source_location>
#include <string_view>
#include <utility> // IWYU pragma: keep - false positive due to an inactive macro

namespace util::detail {

[[noreturn]] void do_unreachable(
    ::std::string_view const message,
    ::std::source_location const location =
        ::std::source_location::current()) noexcept;

} // namespace util::detail

/* Debug unreachable with passing an error message and location */
#ifdef UTIL_UNREACHABLE
# error "UTIL_UNREACHABLE macro could not be defined because it is already defined somewhere else"
#elifdef UTIL_DISABLE_DEBUG
# define UTIL_UNREACHABLE(...) ::std::unreachable()
#else
# define UTIL_UNREACHABLE(...) ::util::detail::do_unreachable(__VA_ARGS__)
#endif

#endif /* DDVAMP_UTIL_DEBUG_UNREACHABLE_HPP_INCLUDED_ */
