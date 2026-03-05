//
// combinator.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_COMBINATOR_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_COMBINATOR_HPP_INCLUDED_ 1

#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/detail/has_cancel.hpp>
#include <exe/future2/detail/has_continue.hpp>
#include <exe/future2/model/demand.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/trait/combine_step.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/trait/value_type.hpp>

namespace exe::future::concepts {

template <typename Step, typename InputType>
concept CombineStep =
    detail::HasContinue<Step, InputType> && detail::HasCancel<Step>;

template <typename C, typename InputType>
concept Combinator =
    ValidInput<InputType, C> &&
    FutureValue<trait::ValueType<C, InputType>> &&
    CombineStep<trait::CombineStep<C,
                       InputType,
                       Demand<trait::ValueType<C, InputType>>>,
                InputType>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_COMBINATOR_HPP_INCLUDED_ */
