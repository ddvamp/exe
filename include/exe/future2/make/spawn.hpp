//
// spawn.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MAKE_SPAWN_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MAKE_SPAWN_HPP_INCLUDED_ 1

#include <exe/unit.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/thunk/make/ready.hpp>
#include <exe/future2/thunk/seq/via.hpp>
#include <exe/future2/thunk/seq/map.hpp>

#include <exe/runtime/inline.hpp>

#include <utility>

namespace exe::future {

template <typename Fn>
Thunk<thunk::Ready<Unit>, thunk::Via, thunk::Map<Fn>> Spawn(Scheduler &where,
                                                            Fn fn) {
  return Thunk(thunk::Ready(Unit{}),
               thunk::Via(where),
               thunk::Map(::std::move(fn)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_SPAWN_HPP_INCLUDED_ */
