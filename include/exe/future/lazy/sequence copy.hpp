#include "index.hpp"

namespace exe::future::lazy {

namespace trait {

template <typename Thunk, typename Consumer>
using Computation = Thunk::template Computation<Consumer>;

template <typename Thunk, typename Consumer>
using ProxiedComputation = Computation<Thunk,
                                       core::Proxy<ValueOf<Thunk>,
                                                   Consumer>>;

} // namespace trait

namespace thunk {

namespace detail {

template <typename Thunk>
struct SequenceHelper {
  template <typename Consumer>
  struct Step : Consumer,
                trait::ProxiedComputation<Thunk, Consumer> {
    using Producer = trait::ProxiedComputation<Thunk, Consumer>;
    using Proxy = core::Proxy<trait::ValueOf<Thunk>, Consumer>;

    template <typename ...Args>
    requires (::std::is_nothrow_constructible_v<Consumer, Args...> &&
              ::std::is_nothrow_constructible_v<Producer, Thunk, Proxy>)
    Step(Thunk &&t, Args &&...args) noexcept
        : Consumer(::std::forward<Args>(args)...)
        , Producer(::std::move(t),
                   core::Proxy(static_cast<Consumer &>(*this))) {}
  };
};

template <typename Thunk>
struct SequenceMaker {
  template <typename Consumer>
  using Step = SequenceHelper<Thunk>::template Step<Consumer>;

  using ValueType = trait::ValueOf<Thunk>;

  template <typename Consumer>
  class Computation : private Step<Consumer> {
   private:
    using Base = Step<Consumer>;

   public:
    using Base::Base;
    using Base::Start;
  };
};

template <typename Thunk>
struct SequenceCombinator {
  template <typename Consumer>
  using Step = SequenceHelper<Thunk>::template Step<Consumer>;

  template <typename>
  inline static constexpr bool ValidInput = true;

  template <typename>
  using ValueType = trait::ValueOf<Thunk>;

  template <typename InputType, typename Consumer>
  struct Continuation : private Step<Consumer> {
   private:
    using Base = Step<Consumer>;

   public:
    using Base::Base;

    void Continue(InputType &&, State s) && noexcept {
      static_cast<Base &&>(*this).Start(s.sched);
    }

    using Base::Cancel;
    using Base::CancelSource;
  };
};

} // namespace detail

template <typename ...>
struct Traits {};

template <typename First, typename ...Rest>
class Sequence {
 private:
  using Traits = thunk::Traits<detail::SequenceMaker<First>,
                               detail::SequenceCombinator<Rest>...>;

  ::std::tuple<First, Rest...> thunks_;

 public:
  Sequence(First &&f, Rest &&...rs) noexcept
      : thunks_{::std::move(f), ::std::move(rs)...} {}

  template <typename Consumer>
  class Computation : private trait::Computation<Traits, Consumer> {
   private:
    using Base = trait::Computation<Traits, Consumer>;

   public:
    template <typename ...Args>
    requires (::std::is_nothrow_constructible_v<Base, First, Rest..., Args...>)
    Computation(Sequence &&s, Args &&...args) noexcept
        : Computation(::std::make_index_sequence<1 + sizeof...(Rest)>{},
                      ::std::move(s),
                      ::std::forward<Args>(args)...) {}

    using Base::Start;

   private:
    template <::std::size_t ...Is, typename ...Args>
    Computation(::std::index_sequence<Is...>, Sequence &&s, Args &&...args)
        : Base(::std::get<Is>(::std::move(s).thunks_)...,
               ::std::forward<Args>(args)...) {}
  };
};

////////////////////////////////////////////////////////////////////////////////

// template <typename Data>
// struct Step {
//   struct Payload {
//     Data data;
//   };

//   Payload p;

//   template <typename InputType>
//   inline static constexpr bool ValidInput = true;

//   template <typename InputType>
//   using ValueType = InputType;

//   template <typename InputType, typename Consumer>
//   class Continuation : private Payload,
//                        private Consumer {
//    public:
//     template <::util::rvalue_deduced ...Args>
//     requires (::std::is_constructible_v<Consumer, Args...>)
//     Continuation(Step &&s, Args &&...args)
//         : Payload(::std::move(s).p)
//         , Consumer(::std::move(args)...) {}

//     /* Continuation */

//     void Continue(InputType &&v, State s) && noexcept {
//       // use payload
//       static_cast<Consumer &&>(*this).Continue(::std::move(v), s);
//     }

//     using Consumer::Cancel;
//     using Consumer::CancelSource;
//   };
// };

} // namespace thunk

} // namespace exe::future::lazy
