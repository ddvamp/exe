//
// strand.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_STRAND_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_STRAND_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/refer/ref.hpp>

namespace exe::runtime {

/**
 *  A scheduler decorator that allows to serialize asynchronous critical sections
 *  without using explicit locks. Instead of "moving the lock" between threads,
 *  it moves critical sections, thereby allowing the data to be in cache
 *  all the time. Sending critical sections is wait-free except for launching
 *  new critical sections of the strand itself
 */
class Strand final : public task::ISafeScheduler {
 private:
  class Impl;
  ::util::ref<Impl> impl_;

 public:
  ~Strand();

  Strand(Strand const &) = delete;
  void operator= (Strand const &) = delete;

  Strand(Strand &&) = delete;
  void operator= (Strand &&) = delete;

 public:
  explicit Strand(ISafeScheduler &underlying);

  [[nodiscard]] ISafeScheduler &GetUnderlying() const noexcept;

  void Submit(task::TaskBase *critical_section) noexcept override;
};

} // namespace exe::runtime

#endif /* DDVAMP_EXE_RUNTIME_STRAND_HPP_INCLUDED_ */
