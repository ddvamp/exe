//
// maker_value.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_MAKER_VALUE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_MAKER_VALUE_HPP_INCLUDED_ 1

#include <util/type_traits.hpp>

namespace exe::future::core::concepts {

template <typename Fn>
concept MakerValue =
    ::std::is_object_v<Fn> &&
    !::util::is_qualified_v<Fn> &&
    ::std::is_nothrow_destructible_v<Fn> &&
    ::std::is_nothrow_move_constructible_v<Fn> &&
    ::std::is_invocable_v<Fn &&>;

} // namespace exe::future::core::concepts

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_MAKER_VALUE_HPP_INCLUDED_ */
