//
// demand.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_DEMAND_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_DEMAND_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/model/state.hpp>

namespace exe::future {

template <typename InputType>
struct Demand {
  void Continue(InputType &&, State) && noexcept;
  void Cancel(State) && noexcept;
  cancel::CancelSource &CancelSource() & noexcept;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MODEL_DEMAND_HPP_INCLUDED_ */
