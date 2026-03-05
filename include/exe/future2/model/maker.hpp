//
// maker.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_MAKER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_MAKER_HPP_INCLUDED_ 1

#include <exe/future2/detail/has_start.hpp>
#include <exe/future2/model/demand.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/trait/make_step.hpp>
#include <exe/future2/trait/value_of.hpp>

namespace exe::future::concepts {

template <typename Step>
concept MakeStep = detail::HasStart<Step>;

template <typename M>
concept Maker =
    FutureValue<trait::ValueOf<M>> &&
    MakeStep<trait::MakeStep<M, Demand<trait::ValueOf<M>>>>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_MODEL_MAKER_HPP_INCLUDED_ */
