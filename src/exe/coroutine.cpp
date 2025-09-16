//
// coroutine.cpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/fiber/core/coroutine.hpp>

#include <util/debug/assert.hpp>

#include <format>
#include <string_view>
#include <utility>

namespace exe::fiber {

namespace {

[[nodiscard, maybe_unused]] ::std::string_view ToStringView(
    Coroutine::Status const status) noexcept {
  using enum Coroutine::Status;
  using namespace std::string_view_literals;

  switch (status) {
  case kInactive:
    return "inactive"sv;

  case kActive:
    return "active"sv;

  case kCompleted:
    return "completed"sv;

  default: [[unlikely]]
    return "unknown"sv;
  };
}

} // namespace

Coroutine::Coroutine(Body &&body, ::util::memory_view stack) noexcept
    : body_(::std::move(body))
    , context_(stack, this) {}

bool Coroutine::IsCompleted() const noexcept {
  return status_ == kCompleted;
}

void Coroutine::Resume() noexcept {
  ChangeStatus(kInactive, kActive);
  context_.SwitchToSaved();
}

void Coroutine::Suspend() noexcept {
  ChangeStatus(kActive, kInactive);
  context_.SwitchToSaved();
}

void Coroutine::Complete() noexcept {
  ChangeStatus(kActive, kCompleted);
  context_.ExitToSaved();
}

/* virtual */ void Coroutine::DoRun() noexcept {
  ::std::move(body_)();
  Complete();
}

void Coroutine::ChangeStatus([[maybe_unused]] Status const from,
                             Status const to) noexcept {
  UTIL_ASSERT(status_ == from, ::std::format(
      "Wrong coroutine status: expected = {}, actual = {}",
      ToStringView(from), ToStringView(status_)));
  status_ = to;
}

} // namespace exe::fiber
