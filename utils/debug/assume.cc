// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <cstdlib>
#include <iostream>

#include "utils/debug/assume.h"
#include "utils/string_builder.h"

namespace utils::detail {

void do_assume(::std::string_view expr, ::std::string_view msg, 
	::std::source_location loc) noexcept
{
	string_builder os(1024);
	os
		<< "Assumption '"
		<< expr
		<< "' is wrong at "
		<< loc.file_name()
		<< ':'
		<< loc.line()
		<< ": "
		<< loc.function_name()
		<< " with message: "
		<< msg
		<< "\nAborting!\n";

	::std::cerr << os.view() << ::std::flush;

	::std::abort();
}

} // namespace utils::detail
