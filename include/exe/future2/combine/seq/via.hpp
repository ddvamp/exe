//
// via.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_COMBINE_SEQ_VIA_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_COMBINE_SEQ_VIA_HPP_INCLUDED_ 1

#include <exe/future2/pipe.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk/seq/via.hpp>

namespace exe::future {

inline auto Via(Scheduler &where) {
  return pipe::Pipe{thunk::Via(where)};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_COMBINE_SEQ_VIA_HPP_INCLUDED_ */
