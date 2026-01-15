//
// body.hpp
// ~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_BODY_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_BODY_HPP_INCLUDED_ 1

#include <functional>

namespace exe::fiber {

using Body = ::std::move_only_function<void() && noexcept>;

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_BODY_HPP_INCLUDED_ */
