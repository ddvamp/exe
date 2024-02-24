// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "abort.hpp"

#include <cstdlib>
#include <format>
#include <iostream>

namespace utils {

void abort(::std::string_view const msg,
	::std::source_location const loc) noexcept {
	::std::cerr << ::std::format("Error at {}:{}: {} with message '{}'. "
		"Abort!\n", loc.file_name(), loc.line(),
		loc.function_name(), msg) << ::std::flush;

	::std::abort();
}

} // namespace utils
