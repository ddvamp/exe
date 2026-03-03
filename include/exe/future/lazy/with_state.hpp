#include "map.hpp"

namespace exe::future::lazy {

namespace thunk {

template <typename Producer>
class [[nodiscard]] WithState {
 private:
  Producer prod_;
  State state_;

 public:
  ~WithState() = default;

  WithState(WithState const &) = delete;
  void operator= (WithState const &) = delete;

  WithState(WithState &&) = default;
  void operator= (WithState &&) = delete;

 public:
  using ValueType = trait::ValueOf<Producer>;

  WithState(Producer &&p, State s) : prod_(::std::move(p)), state_(s) {}

  template <typename Consumer>
  class [[nodiscard]] Continuation {
   private:
    Consumer cons_;
    State state_;

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = default;
    void operator= (Continuation &&) = delete;

   public:
    Continuation(Consumer &&c, State s)
        : cons_(::std::forward<Consumer>(c))
        , state_(s) {}

    void Continue(ValueType &&val, State) && noexcept {
      ::std::move(cons_).Continue(::std::move(val), state_);
    }

    void Failure(Error &&err, State) && noexcept {
      ::std::move(cons_).FailureAware().Failure(::std::move(err), state_);
    }

    void Cancel(State) && noexcept {
      ::std::move(cons_).CancelAware().Cancel(state_);
    }

    /**/

    auto &&FailureAware() && noexcept {
      return ::std::move(*this);
    }

    auto &&CancelAware() && noexcept {
      return ::std::move(*this);
    }

    auto &CancelSource() & noexcept {
      return cons_.CancelSource();
    }
  };

  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return ::std::move(prod_).Materialize(Continuation<Consumer>(
                                              ::std::forward<Consumer>(c),
                                              state_));
  }
};

} // namespace thunk

} // namespace exe::future::lazy
