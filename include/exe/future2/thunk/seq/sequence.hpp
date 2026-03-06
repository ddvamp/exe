//
// sequence.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_SEQ_SEQUENCE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_SEQ_SEQUENCE_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/trait/make_step.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <util/concepts.hpp>

#include <utility>

namespace exe::future::thunk {

// [TODO]: ?util
template <typename ...Ts>
using Tail = Ts...[sizeof...(Ts) - 1];

namespace detail {

template <typename Thunk>
struct SequenceMaker {
  Thunk t_;

  template <typename Consumer>
  using Step = trait::MakeStep<Thunk, Consumer>;

  using ValueType = trait::ValueOf<Thunk>;

  template <concepts::Consumer<ValueType> Consumer>
  struct MakeStep : private Step<Consumer> {
    MakeStep(Consumer &&c, SequenceMaker &sm) noexcept
        : Step<Consumer>(::std::forward<Consumer>(c), sm.t_) {}

    using Step<Consumer>::Start;
  };
};

template <typename Thunk>
struct SequenceCombinator {
  Thunk t_;

  template <typename Consumer>
  using Step = trait::MakeStep<Thunk, Consumer>;

  template <typename InputType>
  inline static constexpr bool ValidInput =
      future::concepts::FutureValue<InputType>;

  template <future::concepts::ValidInput<SequenceCombinator>>
  using ValueType = trait::ValueOf<Thunk>;

  template <future::concepts::ValidInput<SequenceCombinator> InputType,
            concepts::Consumer<ValueType<InputType>> Consumer>
  struct CombineStep : private Step<Consumer> {
    CombineStep(Consumer &&c, SequenceCombinator &sm) noexcept
        : Step<Consumer>(::std::forward<Consumer>(c), sm.t_) {}

    void Continue(InputType &&, State s) && noexcept {
      static_cast<Step<Consumer> &&>(*this).Start(s.sched);
    }
  };
};

} // namespace

template <concepts::Thunk First, concepts::Thunk ...Rest>
requires (sizeof...(Rest) != 0)
class Sequence {
 private:
  using Thunk = future::Thunk<detail::SequenceMaker<First>,
                              detail::SequenceCombinator<Rest>...>;

  template <typename Consumer>
  using Step = trait::MakeStep<Thunk, Consumer>;

  Thunk thunk_;

 public:
  ~Sequence() = default;

  Sequence(Sequence const &) = delete;
  void operator= (Sequence const &) = delete;

  // Move-out only
  Sequence(Sequence &&) = default;
  void operator= (Sequence &&) = delete;

 public:
  Sequence(First &&f, Rest &&...rs) noexcept
      : thunk_({::std::move(f)}, {::std::move(rs)}...) {}

  using ValueType = trait::ValueOf<Thunk>;

  template <concepts::Consumer<ValueType> Consumer>
  struct MakeStep : private Step<Consumer> {
    MakeStep(Consumer &&c, Sequence &s) noexcept
        : Step<Consumer>(::std::forward<Consumer>(c), s.thunk_) {}

    using Step<Consumer>::Start;
  };
};

} // namespace exe::future::thunk

namespace exe::future {

// [TODO]: Move to future/combine
template <::util::rvalue_deduced ...Ts>
inline Thunk<thunk::Sequence<Ts...>> Sequence(Ts &&...ts) noexcept {
  return Thunk(thunk::Sequence(::std::move(ts)...));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_SEQUENCE_HPP_INCLUDED_ */
