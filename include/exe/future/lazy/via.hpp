#include "ready.hpp"

namespace exe::future::lazy {

namespace thunk {

// [TODO]: concept Thunk
template <typename Producer>
class [[nodiscard]] Via {
 private:
  Producer prod_;
  Scheduler *sched_;

 public:
  ~Via() = default;

  Via(Via const &) = delete;
  void operator= (Via const &) = delete;

  Via(Via &&) = default;
  void operator= (Via &&) = delete;

 public:
  using ValueType = trait::ValueOf<Producer>;

  Via(Producer &&p, Scheduler &s) : prod_(::std::move(p)), sched_(&s) {}

  // [TODO]: ?UpdateState/StateUpdater/ModifyState (lambda), without this Continuation
  template <typename Consumer>
  class [[nodiscard]] Continuation {
   private:
    Consumer cons_;
    Scheduler *sched_;

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = default;
    void operator= (Continuation &&) = delete;

   public:
    Continuation(Consumer &&c, Scheduler *s)
        : cons_(::std::forward<Consumer>(c))
        , sched_(s) {}

    void Continue(ValueType &&val, State st) && noexcept {
      st.sched = sched_;
      ::std::move(cons_).Continue(::std::move(val), st);
    }

    void Failure(Error &&err, State st) && noexcept {
      st.sched = sched_;
      ::std::move(cons_).FailureAware().Failure(::std::move(err), st);
    }

    void Cancel(State st) && noexcept {
      st.sched = sched_;
      ::std::move(cons_).CancelAware().Cancel(st);
    }

    /**/

    // [TODO]: ?concept FailureAware
    auto &&FailureAware() && noexcept {
      return ::std::move(*this);
    }

    // [TODO]: ?concept CancelAware
    auto &&CancelAware() && noexcept {
      return ::std::move(*this);
    }

    // [TODO]: ?concept CancelSource, concrete type, interface
    auto &CancelSource() & noexcept {
      return cons_.CancelSource();
    }
  };

  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return ::std::move(prod_).Materialize(Continuation<Consumer>(
                                              ::std::forward<Consumer>(c),
                                              sched_));
  }
};

} // namespace thunk

} // namespace exe::future::lazy
