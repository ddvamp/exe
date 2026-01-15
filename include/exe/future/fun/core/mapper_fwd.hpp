//
// mapper_fwd.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_FWD_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_FWD_HPP_INCLUDED_ 1

#include <exe/future/fun/core/concept/mapper_value.hpp>

namespace exe::future::core {

template <concepts::MapperValue Fn>
class Mapper;

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_FWD_HPP_INCLUDED_ */
