//
// concepts.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_CONCEPTS_HPP_INCLUDED_
#define DDVAMP_UTIL_CONCEPTS_HPP_INCLUDED_ 1

#include <util/type_traits.hpp>

#include <concepts> // IWYU pragma: export

namespace util {

/**
 *  For type deduction from rvalue expressions only.
 *  Prohibits the use of lvalue as an argument for forwarding reference (T &&)
 */
template <typename T>
concept rvalue_deduced = !::std::is_lvalue_reference_v<T>;

} // namespace util

#endif /* DDVAMP_UTIL_CONCEPTS_HPP_INCLUDED_ */
