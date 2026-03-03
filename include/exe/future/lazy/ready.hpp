#include "lazy.hpp"

namespace exe::future::lazy {

namespace thunk {

// [TODO]: ?concept FutureValue
template <typename Value>
class [[nodiscard]] Ready {
 private:
  Value val_;

 public:
  ~Ready() = default;

  Ready(Ready const &) = delete;
  void operator= (Ready const &) = delete;

  Ready(Ready &&) = default;
  void operator= (Ready &&) = delete;

 public:
  using ValueType = Value;

  explicit Ready(Value &&v) : val_(::std::move(v)) {}

  // [TODO]: concept Continuation
  template <typename Consumer>
  class [[nodiscard]] Computation {
   private:
    Consumer cons_;
    Value val_;

   public:
    ~Computation() = default;

    Computation(Computation const &) = delete;
    void operator= (Computation const &) = delete;

    Computation(Computation &&) = default;
    void operator= (Computation &&) = delete;

   public:
    Computation(Consumer &&c, Value &&v)
        : cons_(::std::forward<Consumer>(c))
        , val_(::std::move(v)) {}

    void Start(Scheduler &s) && noexcept {
      ::std::move(cons_).Continue(::std::move(val_), State(&s));
    }
  };

  // [TODO]: concept Computation
  // [TODO]: ?not rvalue_reference
  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return Computation<Consumer>(::std::forward<Consumer>(c),
                                 ::std::move(val_));
  }
};

} // namespace thunk

template <typename V>
auto Ready(V v) {
  return thunk::Ready(::std::move(v));
}

} // namespace exe::future::lazy
