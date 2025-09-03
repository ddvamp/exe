//
// id.hpp
// ~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_ID_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_ID_HPP_INCLUDED_ 1

#include <cstdint>

namespace exe::fiber {

using FiberId = ::std::uint64_t;

inline constexpr FiberId kInvalidFiberId = -1;

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_ID_HPP_INCLUDED_ */
