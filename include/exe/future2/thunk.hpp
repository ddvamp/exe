//
// thunk.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ 1

#include <exe/future2/execution_core.hpp>
#include <exe/future2/thunk_data.hpp>
#include <exe/future2/thunk_traits.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/model/thunk_resource.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <util/concepts.hpp>

#include <utility>

namespace exe::future {

namespace concepts {

template <typename Maker, typename ...Combinators>
concept CorrectPipeline =
    !detail::ErroneousTraits<Traits<Maker, Combinators...>>;

} // namespace concepts

template <concepts::ThunkResource Maker,
          concepts::ThunkResource ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>)
class [[nodiscard]] Thunk {
 private:
  template <concepts::ThunkResource M, concepts::ThunkResource ...Cs>
  requires (concepts::CorrectPipeline<M, Cs...>)
  friend class Thunk;

  using Data = ThunkData<Maker, Combinators...>;
  using Traits = Traits<Maker, Combinators...>;

  template <typename ...Cs>
  using ExtendedThunk = Thunk<Maker, Combinators..., Cs...>;

  Data data_;

 public:
  ~Thunk() = default;

  Thunk(Thunk const &) = delete;
  void operator= (Thunk const &) = delete;

  // Move-out only
  Thunk(Thunk &&) = default;
  void operator= (Thunk &&) = delete;

 public:
  explicit Thunk(Maker &&m, Combinators &&...cs) noexcept
      : data_{{::std::move(m)}, {::std::move(cs)}...} {}

  using ValueType = trait::ValueOf<Traits>;

  template <concepts::Consumer<ValueType> Consumer>
  class MakeStep;

  template <concepts::Consumer<ValueType> Consumer>
  class Computation;

  template <typename Consumer>
  inline Computation<Consumer> Materialize(Consumer &&c) && noexcept {
    return Computation<Consumer>(::std::forward<Consumer>(c),
                                 ::std::move(*this));
  }

  template <::util::rvalue_deduced ...Cs>
  inline ExtendedThunk<Cs...> Extend(Cs &&...c) && noexcept {
    return ::std::move(data_).Extend(::std::move(c)...);
  }

  template <typename ...Cs>
  inline ExtendedThunk<Cs...> Extend(ThunkData<Cs...> &&d) && noexcept {
    return ::std::move(data_).Extend(::std::move(d));
  }

 private:
  Thunk(Data &&d) noexcept : data_(::std::move(d)) {}
};

template <typename ...Ts>
inline constexpr bool trait::Thunk<Thunk<Ts...>> = true;

////////////////////////////////////////////////////////////////////////////////

template <concepts::ThunkResource Maker,
          concepts::ThunkResource ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>)
template <concepts::Consumer<
              trait::ValueOf<Thunk<Maker, Combinators...>>> Consumer>
class Thunk<Maker, Combinators...>::MakeStep
    : private ExecutionCore<Consumer, Maker, Combinators...> {
 private:
  using Base = MakeStep::ExecutionCore;

 public:
  MakeStep(Consumer &&c, Thunk &t) noexcept
      : Base(::std::forward<Consumer>(c), t.data_) {}

  using Base::Start;
};

////////////////////////////////////////////////////////////////////////////////

template <concepts::ThunkResource Maker,
          concepts::ThunkResource ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>)
template <concepts::Consumer<
              trait::ValueOf<Thunk<Maker, Combinators...>>> Consumer>
class Thunk<Maker, Combinators...>::Computation
    : private Thunk,
      private ExecutionCore<Consumer, Maker, Combinators...> {
 private:
  using Base = Computation::ExecutionCore;

 public:
  ~Computation() = default;

  Computation(Computation const &) = delete;
  void operator= (Computation const &) = delete;

  Computation(Computation &&) = delete;
  void operator= (Computation &&) = delete;

 public:
  Computation(Consumer &&c, Thunk &&t) noexcept
      : Thunk(::std::move(t))
      , Base(::std::forward<Consumer>(c), data_) {}

  using Base::Start;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ */
