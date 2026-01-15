//
// abort.cpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <util/abort.hpp>

#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>
#include <string_view>

namespace util {

void abort(::std::string_view const msg, ::std::source_location const loc)
    noexcept {
  try {
    auto const output = ::std::format(
        "Error at {}:{}: {} with message '{}'. Abort!\n",
        loc.file_name(), loc.line(), loc.function_name(), msg);
    ::std::cerr << output << ::std::flush;
  } catch (...) {
    // [TODO]: ?Backup logging
  }

  ::std::abort();
}

} // namespace util
