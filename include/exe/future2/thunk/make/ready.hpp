//
// ready.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_MAKE_READY_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_MAKE_READY_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>

#include <utility>

namespace exe::future::thunk {

template <concepts::FutureValue Value>
class [[nodiscard]] Ready {
 private:
  Value val_;

 public:
  ~Ready() = default;

  Ready(Ready const &) = delete;
  void operator= (Ready const &) = delete;

  // Move-out only
  Ready(Ready &&) = default;
  void operator= (Ready &&) = delete;

 public:
  explicit Ready(Value v) noexcept : val_(::std::move(v)) {}

  using ValueType = Value;

  template <concepts::Continuation<ValueType> Consumer>
  struct MakeStep {
    Consumer cons_;
    Ready &data_;

    MakeStep(Consumer &&c, Ready &r)
        : cons_(::std::forward<Consumer>(c))
        , data_(r) {}

    void Start(Scheduler &s) && noexcept {
      ::std::move(cons_).Continue(::std::move(data_).val_, {.sched = s});
    }
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_MAKE_READY_HPP_INCLUDED_ */
