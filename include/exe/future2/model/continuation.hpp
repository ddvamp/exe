//
// continuation.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_CONTINUATION_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_CONTINUATION_HPP_INCLUDED_ 1

#include <exe/future2/detail/has_cancel.hpp>
#include <exe/future2/detail/has_cancel_source.hpp>
#include <exe/future2/detail/has_continue.hpp>

namespace exe::future::concepts {

template <typename Consumer, typename InputType>
concept Continuation =
    detail::HasContinue<Consumer, InputType> &&
    detail::HasCancel<Consumer> &&
    detail::HasCancelSource<Consumer>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_CONTINUATION_HPP_INCLUDED_ */
