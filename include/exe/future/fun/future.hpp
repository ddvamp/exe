//
// future.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

// Umbrella header

#ifndef DDVAMP_EXE_FUTURE_FUN_FUTURE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_FUTURE_HPP_INCLUDED_ 1

#include <exe/future/fun/combine/concur/all.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/concur/first.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/after.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/flat_map.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/flatten.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/inline.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/invoke_with.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/map.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/recover.hpp> // IWYU pragma: export
#include <exe/future/fun/combine/seq/via.hpp> // IWYU pragma: export
#include <exe/future/fun/make/failure.hpp> // IWYU pragma: export
#include <exe/future/fun/make/just.hpp> // IWYU pragma: export
#include <exe/future/fun/make/spawn.hpp> // IWYU pragma: export
#include <exe/future/fun/make/value.hpp> // IWYU pragma: export
#include <exe/future/fun/result/error.hpp> // IWYU pragma: export
#include <exe/future/fun/result/result.hpp> // IWYU pragma: export
#include <exe/future/fun/result/unit.hpp> // IWYU pragma: export
#include <exe/future/fun/terminate/detach.hpp> // IWYU pragma: export
#include <exe/future/fun/terminate/get.hpp> // IWYU pragma: export

#endif /* DDVAMP_EXE_FUTURE_FUN_FUTURE_HPP_INCLUDED_ */
