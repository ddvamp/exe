//
// future_value.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_FUTURE_VALUE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_FUTURE_VALUE_HPP_INCLUDED_ 1

#include <exe/future2/model/thunk_resource.hpp>

namespace exe::future::concepts {

template <typename V>
concept FutureValue = ThunkResource<V>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_FUTURE_VALUE_HPP_INCLUDED_ */
