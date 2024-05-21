// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_STRAND_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_STRAND_HPP_INCLUDED_ 1

#include <utils/refer/ref.hpp>

#include "task/scheduler.hpp"

namespace exe::sched {

// A scheduler decorator that allows to serialize asynchronous critical sections
// without using explicit locks. Instead of moving the "lock" between threads,
// it moves critical sections, thereby allowing the data to be in cache
// all the time. Sending critical sections is wait-free except for launching
// new critical sections of the strand itself
class Strand final : public task::INothrowScheduler {
 private:
  using IScheduler = INothrowScheduler;

  class Impl;
  ::utils::ref<Impl> impl_;

 public:
  ~Strand();

  Strand(Strand const &) = delete;
  void operator= (Strand const &) = delete;

  Strand(Strand &&) = delete;
  void operator= (Strand &&) = delete;

 public:
  explicit Strand(IScheduler &underlying);

  [[nodiscard]] IScheduler &GetUnderlying() const noexcept;

  void Submit(task::TaskBase *critical_section) noexcept override;
};

}  // namespace exe::sched

#endif /* DDVAMP_EXE_SCHED_STRAND_HPP_INCLUDED_ */
