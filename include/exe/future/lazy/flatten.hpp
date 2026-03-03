#include "with_state.hpp"

namespace exe::future::lazy {

namespace thunk {

template <typename Producer>
class [[nodiscard]] Flatten {
 private:
  Producer prod_;

  using Future = trait::ValueOf<Producer>;

 public:
  ~Flatten() = default;

  Flatten(Flatten const &) = delete;
  void operator= (Flatten const &) = delete;

  Flatten(Flatten &&) = default;
  void operator= (Flatten &&) = delete;

 public:
  using ValueType = trait::ValueOf<Future>;

  explicit Flatten(Producer &&p) : prod_(::std::move(p)) {}

  template <typename Consumer>
  class [[nodiscard]] Continuation {
   private:
    Consumer cons_;
    ::std::optional<trait::Materialize<WithState<Future>, Consumer &>> prod_;

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = default;
    void operator= (Continuation &&) = delete;

   public:
    explicit Continuation(Consumer &&c) : cons_(::std::forward<Consumer>(c)) {}

    // Equivalent cons.Continue(Await(fut), st);
    void Continue(Future &&fut, State st) && noexcept {
      prod_.emplace(WithState(::std::move(fut), st).Materialize(cons_));
      (*::std::move(prod_)).Start(*st.sched);
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
  };

  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return ::std::move(prod_).Materialize(Continuation<Consumer>(
                                              ::std::forward<Consumer>(c)));
  }
};

} // namespace thunk

} // namespace exe::future::lazy
