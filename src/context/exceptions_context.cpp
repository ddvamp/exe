//
// exceptions_context.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <context/exceptions_context.hpp>

#include <cstring>

namespace __cxxabiv1 {

extern "C" struct __cxa_eh_globals *__cxa_get_globals() noexcept;

} // namespace __cxxabiv1

namespace context {

void ExceptionsContext::SwitchTo(ExceptionsContext &target) noexcept {
  auto *current = ::__cxxabiv1::__cxa_get_globals();

  State tmp;

  // Prevent aliasing
  ::std::memcpy(tmp, target.state_, sizeof(State));
  ::std::memcpy(state_, current, sizeof(State));
  ::std::memcpy(current, tmp, sizeof(State));
}

} // namespace context
