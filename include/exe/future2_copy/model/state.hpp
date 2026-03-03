//
// state.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_STATE_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>

namespace exe::future {

struct State {
  Scheduler &sched;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MODEL_STATE_HPP_INCLUDED_ */
