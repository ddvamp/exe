//
// result.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RESULT_RESULT_HPP_INCLUDED_
#define DDVAMP_EXE_RESULT_RESULT_HPP_INCLUDED_ 1

#include <exe/result/trait/result.hpp>

#include <expected>

namespace exe {

template <typename Value, typename Error>
using Result = ::std::expected<Value, Error>;

template <typename V, typename E>
inline constexpr bool trait::Result<Result<V, E>> = true;

} // namespace exe

#endif /* DDVAMP_EXE_RESULT_RESULT_HPP_INCLUDED_ */
