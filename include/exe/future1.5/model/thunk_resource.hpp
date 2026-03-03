//
// thunk_resource.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_THUNK_RESOURCE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_THUNK_RESOURCE_HPP_INCLUDED_ 1

#include <exe/future2/concept/async_safe.hpp>

#include <util/type_traits.hpp>

namespace exe::future::concepts {

template <typename R>
concept ThunkResource =
    AsyncSafe<R> && // implies !::std::is_array_v<R>
    ::std::is_object_v<R> &&
    !::util::is_qualified_v<R>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_THUNK_RESOURCE_HPP_INCLUDED_ */
