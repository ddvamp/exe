//
// unreachable.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <util/debug/unreachable.hpp>

#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>
#include <string_view>

namespace util::detail {

void do_unreachable(::std::string_view const msg,
                    ::std::source_location const loc) noexcept {
  try {
    auto const output = ::std::format(
        "Debug error! Unreachable point has been reached at "
        "{}:{}: {} with message '{}'. Abort!\n",
        loc.file_name(), loc.line(), loc.function_name(), msg);
    ::std::cerr << output << ::std::flush;
  } catch (...) {
    // [TODO]: ?Backup logging
  }

  ::std::abort();
}

} // namespace util::detail
