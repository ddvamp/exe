//
// mapper.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_MAPPER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_MAPPER_HPP_INCLUDED_ 1

#include <exe/future/fun/core/mapper_fwd.hpp>

namespace exe::future::core::concepts {

namespace detail {

template <typename>
inline constexpr bool IsMapperImpl = false;

template <typename T>
inline constexpr bool IsMapperImpl<Mapper<T>> = true;

} // namespace detail

template <typename T>
concept Mapper = detail::IsMapperImpl<T>;

} // namespace exe::future::core::concepts

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_MAPPER_HPP_INCLUDED_ */
