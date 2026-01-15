//
// via.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_VIA_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_VIA_HPP_INCLUDED_ 1

#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/future/fun/type/scheduler.hpp>

#include <utility>

namespace exe::future {

namespace pipe {

class [[nodiscard]] Via : private core::Operator {
 private:
  Scheduler &where_;

 public:
  explicit Via(Scheduler &where) : where_(where) {}

  template <typename T>
  Future<T> Apply(SemiFuture<T> f) && {
    return SetScheduler(::std::move(f), where_);
  }
};

} // namespace pipe

inline auto Via(Scheduler &where) {
  return pipe::Via(where);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_VIA_HPP_INCLUDED_ */
