#include "flatten.hpp"

namespace exe::future::lazy {

namespace concepts {

template <typename Consumer, typename Input, ::std::size_t I>
concept IndexedContinue = requires (Consumer c, Input &&val, State st) {
  ::std::move(c).template Continue<I>(::std::move(val), st);
};

template <typename Consumer, ::std::size_t I>
concept IndexedFailure = requires (Consumer c, Error &&err, State st) {
  ::std::move(c).FailureAware().template Failure<I>(::std::move(err), st);
};

template <typename Consumer, ::std::size_t I>
concept IndexedCancel = requires (Consumer c, State st) {
  ::std::move(c).CancelAware().template Cancel<I>(st);
};

} // namespace concepts

namespace thunk {

template <typename Producer, ::std::size_t I>
class [[nodiscard]] Index {
 private:
  Producer prod_;

 public:
  ~Index() = default;

  Index(Index const &) = delete;
  void operator= (Index const &) = delete;

  Index(Index &&) = default;
  void operator= (Index &&) = delete;

 public:
  using ValueType = trait::ValueOf<Producer>;

  explicit Index(Producer &&prod) : prod_(::std::move(prod)) {}

  template <typename Consumer>
  class [[nodiscard]] Continuation {
   private:
    Consumer cons_;

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = default;
    void operator= (Continuation &&) = delete;

   public:
    explicit Continuation(Consumer &&c) : cons_(::std::forward<Consumer>(c)) {
      // [TODO]: Better assert
      static_assert(concepts::IndexedContinue<Consumer, ValueType, I> ||
                    concepts::IndexedFailure<Consumer, I> ||
                    concepts::IndexedCancel<Consumer, I>,
                    "Useless Index");
    }

    void Continue(ValueType &&val, State st) && noexcept {
      if constexpr(concepts::IndexedContinue<Consumer, ValueType, I>) {
        ::std::move(cons_).template Continue<I>(::std::move(val), st);
      } else {
        ::std::move(cons_).Continue(::std::move(val), st);
      }
    }

    void Failure(Error &&err, State st) && noexcept {
      ::std::move(cons_).FailureAware().template Failure<I>(::std::move(err),
                                                            st);
    }

    void Cancel(State st) && noexcept {
      ::std::move(cons_).CancelAware().template Cancel<I>(st);
    }

    /**/

    auto &&FailureAware() && noexcept {
      if constexpr (concepts::IndexedFailure<Consumer, I>) {
        return ::std::move(*this);
      } else {
        return ::std::move(cons_).FailureAware();
      }
    }

    auto &&CancelAware() && noexcept {
      if constexpr (concepts::IndexedCancel<Consumer, I>) {
        return ::std::move(*this);
      } else {
        return ::std::move(cons_).CancelAware();
      }
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
