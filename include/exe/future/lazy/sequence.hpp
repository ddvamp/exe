#include "index.hpp"

namespace exe::future::lazy {

namespace thunk {

template <typename ...Producers>
struct IndexedSequence {
  template <typename Consumer>
  class [[nodiscard]] Computation {
   private:
    Consumer cons_;
    ::std::tuple<Producers...> prod_thunks_;
    ::std::tuple<::std::optional<trait::Materialize<Producers,
                                                    Computation &>>...> prods_;

   public:
    ~Computation() = default;

    Computation(Computation const &) = delete;
    void operator= (Computation const &) = delete;

    Computation(Computation &&) = default;
    void operator= (Computation &&) = delete;

   public:
    Computation(Consumer &&c, Producers &&...ps)
        : cons_(::std::forward<Consumer>(c))
        , prod_thunks_(::std::move(ps)...) {}

    void Start(Scheduler &sched) && noexcept {
      Start<0>(sched);
    }

    template <::std::size_t I>
    void Continue(trait::ValueOf<Producers...[I]> &&val, State st) && noexcept {
      if constexpr (I == sizeof...(Producers) - 1) {
        ::std::move(cons_).Continue(::std::move(val), st);
      } else {
        UTIL_IGNORE(val);
        Start<I + 1>(*st.sched);
      }
    }

    /**/

    auto &&FailureAware() && noexcept {
      return ::std::move(cons_).FailureAware();
    }

    auto &&CancelAware() && noexcept {
      return ::std::move(cons_).CancelAware();
    }

    auto &CancelSource() & noexcept {
      return cons_.CancelSource();
    }

   private:
    template <::std::size_t I>
    void Start(Scheduler &sched) noexcept {
      auto &thunk = ::std::get<I>(prod_thunks_);
      auto &prod = ::std::get<I>(prods_);

      prod.emplace(::std::move(thunk).Materialize(*this));
      (*::std::move(prod)).Start(sched);
    }
  };

  template <typename Consumer>
  static auto Materialize(Consumer &&c, Producers &&...prods) {
    return Computation<Consumer>(::std::forward<Consumer>(c),
                                 ::std::move(prods)...);
  }
};

// [TODO]: in pipe sizeof > 1
template <typename ...Producers>
class [[nodiscard]] Sequence {
 private:
  ::std::tuple<Producers...> prods_;

 public:
  ~Sequence() = default;

  Sequence(Sequence const &) = delete;
  void operator= (Sequence const &) = delete;

  Sequence(Sequence &&) = default;
  void operator= (Sequence &&) = delete;

 public:
  using ValueType = trait::ValueOf<Tail<Producers...>>;

  explicit Sequence(Producers &&...prods) : prods_(::std::move(prods)...) {}

  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return [&]<::std::size_t ...Is>(::std::index_sequence<Is...>) {
      using Seq = IndexedSequence<Index<Producers, Is>...>;
      return Seq::Materialize(::std::forward<Consumer>(c),
                              Index<Producers, Is>(
                                  ::std::get<Is>(::std::move(prods_)))...);
    }(::std::index_sequence_for<Producers...>{});
  }
};

} // namespace thunk

template <::util::rvalue_deduced ...Producers>
requires (sizeof...(Producers) > 1)
auto Sequence(Producers &&...prods) {
  return thunk::Sequence(::std::move(prods)...);
}

} // namespace exe::future::lazy
