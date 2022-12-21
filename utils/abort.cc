#include <cstdlib>
#include <iostream>

#include "utils/abort.h"
#include "utils/string_builder.h"

namespace utils {

void abort(::std::string_view msg, ::std::source_location loc) noexcept
{
	string_builder os(1024);
	os
		<< "Error at "
		<< loc.file_name()
		<< ':'
		<< loc.line()
		<< ": "
		<< loc.function_name()
		<< " with message: "
		<< msg
		<< '\n';

	::std::cerr << os.view() << ::std::flush;

	::std::abort();
}

} // namespace utils
