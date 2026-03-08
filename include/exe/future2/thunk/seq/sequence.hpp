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
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/trait/make_step.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <utility>

namespace exe::future::thunk {

namespace detail {

template <typename Thunk>
struct SequenceMaker {
  Thunk t_;

  using ValueType = trait::ValueOf<Thunk>;

  template <typename Consumer>
  struct MakeStep : private trait::MakeStep<Thunk, Consumer> {
    using Base = trait::MakeStep<Thunk, Consumer>;

    MakeStep(Consumer &&c, SequenceMaker &sm) noexcept
        : Base(::std::forward<Consumer>(c), sm.t_) {}

    using Base::Start;
  };
};

template <typename Thunk>
struct SequenceCombinator {
  Thunk t_;

  template <typename>
  inline static constexpr bool ValidInput = true;

  template <typename>
  using ValueType = trait::ValueOf<Thunk>;

  template <typename InputType, typename Consumer>
  struct CombineStep : private trait::MakeStep<Thunk, Consumer> {
    using Base = trait::MakeStep<Thunk, Consumer>;

    CombineStep(Consumer &&c, SequenceCombinator &sm) noexcept
        : Base(::std::forward<Consumer>(c), sm.t_) {}

    void Continue(InputType &&, State s) && noexcept {
      static_cast<Base &&>(*this).Start(s.sched);
    }
  };
};

} // namespace

template <concepts::Thunk First, concepts::Thunk ...Rest>
requires (sizeof...(Rest) != 0)
class [[nodiscard]] Sequence {
 private:
  using Thunk = Thunk<detail::SequenceMaker<First>,
                      detail::SequenceCombinator<Rest>...>;

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
  struct MakeStep : private trait::MakeStep<Thunk, Consumer> {
    using Base = trait::MakeStep<Thunk, Consumer>;

    MakeStep(Consumer &&c, Sequence &s) noexcept
        : Base(::std::forward<Consumer>(c), s.thunk_) {}

    using Base::Start;
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_SEQUENCE_HPP_INCLUDED_ */
