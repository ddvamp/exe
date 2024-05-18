// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "coroutine.hpp"

#include <format>
#include <string_view>
#include <utility>

#include <utils/abort.hpp>
#include <utils/debug/assert.hpp>

namespace exe::fiber {

namespace {

[[nodiscard, maybe_unused]] ::std::string_view ToStringView(
    Coroutine::Status const status) noexcept {
  switch (status) {
  case Coroutine::Status::active:    return "active";
  case Coroutine::Status::inactive:  return "inactive";
  case Coroutine::Status::completed: return "completed";
  default: UTILS_ABORT("Unexpected Coroutine::Status");
  };
}

}  // namespace

Coroutine::Coroutine(Body &&body, ::utils::memory_view stack) noexcept
    : body_(::std::move(body))
    , context_(stack, this) {}

bool Coroutine::IsCompleted() const noexcept {
  return status_ == Status::completed;
}

void Coroutine::Resume() noexcept {
  ChangeStatus(Status::inactive, Status::active);
  context_.SwitchToSaved();
}

void Coroutine::Suspend() noexcept {
  ChangeStatus(Status::active, Status::inactive);
  context_.SwitchToSaved();
}

void Coroutine::Cancel() noexcept {
  ChangeStatus(Status::active, Status::completed);
  context_.ExitToSaved();
}

/* virtual */ void Coroutine::DoRun() noexcept {
  ::std::move(body_)();
  Cancel();
}

void Coroutine::ChangeStatus(Status const from, Status const to) noexcept {
  UTILS_ASSERT(status_ == from,
      ::std::format("Coroutine is in wrong status: expected = {}, actual = {}",
                    ToStringView(from), ToStringView(status_)));
  status_ = to;
}

}  // namespace exe::fiber
