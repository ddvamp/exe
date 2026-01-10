//
// get.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TERMINATE_GET_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TERMINATE_GET_HPP_INCLUDED_ 1

#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <concurrency/pause.hpp>

#include <atomic>
#include <optional>
#include <utility>

namespace exe::future {

namespace pipe {

class [[nodiscard]] Get : private core::Operator {
 private:
  enum class Phase {
    Init,
    Notified,
    Done
  };
  using enum Phase;

 public:
  template <typename T>
  [[nodiscard]] T Apply(SemiFuture<T> f) && {
    ::std::optional<Result<T>> result;

    ::std::atomic<Phase> phase = Init;

    // To guarantee the expected implementation
    static_assert(::std::atomic<Phase>::is_always_lock_free);

    auto cb = [&](Result<T> &&res) noexcept {
      result.emplace(::std::move(res));

      phase.store(Notified, ::std::memory_order_relaxed);
      phase.notify_all();
      phase.store(Done, ::std::memory_order_release);
    };

    SetCallback(SetScheduler(::std::move(f), runtime::GetInline()), cb);

    phase.wait(Init, ::std::memory_order_relaxed);
    while (phase.load(::std::memory_order_acquire) != Done) {
      ::concurrency::Pause();
    }

    if (result->has_value()) {
      return **::std::move(result);
    }

    ThrowError((*::std::move(result)).error());
  }
};

} // namespace pipe

inline auto Get() noexcept {
  return pipe::Get();
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TERMINATE_GET_HPP_INCLUDED_ */
