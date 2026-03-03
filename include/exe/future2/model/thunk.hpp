//
// thunk.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_THUNK_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_THUNK_HPP_INCLUDED_ 1

namespace exe::future {

namespace trait {

template <typename>
inline constexpr bool Thunk = false;

} // namespace trait

namespace concepts {

template <typename T>
concept Thunk = trait::Thunk<T>;

} // namespace concepts

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MODEL_THUNK_HPP_INCLUDED_ */
