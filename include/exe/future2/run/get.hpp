//
// get.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_RUN_GET_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_RUN_GET_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/type/future.hpp>
#include <exe/runtime/inline.hpp>

#include <concurrency/ack_event.hpp>
#include <util/abort.hpp>
#include <util/debug/assert.hpp>

#include <optional>

namespace exe::future {

namespace detail {

// [TODO]: Move to core
template <concepts::FutureValue V>
class GetConsumer {
 private:
  ::std::optional<V> res_;
  cancel::CancelSource source_;
  ::concurrency::AckEvent event_;

 public:
  void Continue(V &&v, State) && noexcept {
    res_.emplace(::std::move(v));
    source_.RequestCancel(); // [TODO]: ReleaseHandlers
    event_.Fire();
  }

  void Cancel(State) && noexcept {
    UTIL_ABORT("Unexpected upstream cancellation");
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return source_;
  }

  V Get() && noexcept {
    event_.Wait();
    UTIL_ASSERT(res_.has_value(), "Missing result");
    return *::std::move(res_);
  }
};

} // namespace detail

namespace pipe {

struct [[nodiscard]] Get {
  template <concepts::SomeFuture F>
  trait::ValueOf<F> Pipe(F &&f) && {
    detail::GetConsumer<trait::ValueOf<F>> cons;
    auto comp = ::std::move(f).Materialize(cons);
    ::std::move(comp).Start(exe::runtime::GetInline());
    return ::std::move(cons).Get();
  }
};

} // namespace pipe

inline auto Get() noexcept {
  return pipe::Get();
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_RUN_GET_HPP_INCLUDED_ */
