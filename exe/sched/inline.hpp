// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_INLINE_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_INLINE_HPP_INCLUDED_ 1

#include "task/scheduler.hpp"

namespace exe::sched {

// Executes task immediately at place
class [[nodiscard]] Inline : public task::INothrowScheduler {
 public:
  void Submit(task::TaskBase *) noexcept override;
};

class Inline &Inline() noexcept;

}  // namespace exe::sched

#endif /* DDVAMP_EXE_SCHED_INLINE_HPP_INCLUDED_ */
