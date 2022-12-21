#include <cstdlib>
#include <iostream>

#include "utils/assert.h"
#include "utils/string_builder.h"

namespace utils::detail {

void do_assert(::std::string_view expr, ::std::string_view msg, 
	::std::source_location loc) noexcept
{
	string_builder os(1024);
	os
		<< "Assertion '"
		<< expr
		<< "' at "
		<< loc.file_name()
		<< ':'
		<< loc.line()
		<< ": "
		<< loc.function_name()
		<< " failed with message: "
		<< msg
		<< '\n';

	::std::cerr << os.view() << ::std::flush;

	::std::abort();
}

} // namespace utils::detail
