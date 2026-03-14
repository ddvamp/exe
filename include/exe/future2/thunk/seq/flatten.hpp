//
// flatten.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_SEQ_FLATTEN_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_SEQ_FLATTEN_HPP_INCLUDED_ 1

#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/trait/computation.hpp>
#include <exe/future2/trait/make_step.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/type/future.hpp>

#include <optional>
#include <utility>

namespace exe::future::thunk {

class [[nodiscard]] Flatten {
 public:
  ~Flatten() = default;

  Flatten(Flatten const &) = delete;
  void operator= (Flatten const &) = delete;

  // Move-out only
  Flatten(Flatten &&) = default;
  void operator= (Flatten &&) = delete;

 public:
  Flatten() = default;

  template <typename InputType>
  inline static constexpr bool ValidInput = concepts::SomeFuture<InputType>;

  template <concepts::ValidInput<Flatten> InputType>
  using ValueType = trait::ValueOf<InputType>;

  template <concepts::ValidInput<Flatten> InputType,
            concepts::Consumer<ValueType<InputType>> Consumer>
  struct CombineStep {
    using Thunk = InputType;
    using Comp = trait::Computation<Thunk, Consumer &>;

    Consumer cons_;
    ::std::optional<Comp> comp_;

    CombineStep(Consumer &&c, Flatten &) noexcept
        : cons_(::std::forward<Consumer>(c)) {}

    void Continue(Thunk &&t, State s) && noexcept {
      auto &c = comp_.emplace(cons_, ::std::move(t));
      ::std::move(c).Start(s.sched);
    }
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_FLATTEN_HPP_INCLUDED_ */
