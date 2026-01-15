//
// strand.cpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/runtime/strand.hpp>
#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>
#include <internal/strand.hpp>

namespace exe::runtime {

Strand::~Strand() = default;

Strand::Strand(ISafeScheduler &underlying) : impl_(Impl::Create(underlying)) {}

/* virtual */ void Strand::Submit(task::TaskBase *task) noexcept {
  static_assert(noexcept(impl_->Submit(task)),
                "Implementation must be noexcept");
  impl_->Submit(task);
}

task::ISafeScheduler &Strand::GetUnderlying() const noexcept {
  return impl_->GetUnderlying();
}

} // namespace exe::runtime
