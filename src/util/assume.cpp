//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <cstdlib>
#include <iostream>

#include "util/debug/assume.hpp"
#include "util/string_builder.hpp"

namespace util::detail {

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

} // namespace util::detail
