//
// value_type.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_TRAIT_VALUE_TYPE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_TRAIT_VALUE_TYPE_HPP_INCLUDED_ 1

namespace exe::future::trait {

template <typename T, typename InputType>
using ValueType = T::template ValueType<InputType>;

} // namespace exe::future::trait

#endif /* DDVAMP_EXE_FUTURE_TRAIT_VALUE_TYPE_HPP_INCLUDED_ */
