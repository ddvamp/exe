#include "via.hpp"

namespace exe::future::lazy {

namespace thunk {

template <typename Producer, typename Mapper>
class [[nodiscard]] Map {
 private:
  Producer prod_;
  Mapper map_;

  using Input = trait::ValueOf<Producer>;

 public:
  ~Map() = default;

  Map(Map const &) = delete;
  void operator= (Map const &) = delete;

  Map(Map &&) = default;
  void operator= (Map &&) = delete;

 public:
  using ValueType = future::core::trait::AdaptCallResult<Mapper, Input>;

  Map(Producer &&p, Mapper &&m) : prod_(::std::move(p)), map_(::std::move(m)) {}

  template <typename Consumer>
  class [[nodiscard]] Continuation final : private runtime::task::TaskBase {
   private:
    Consumer cons_;
    Mapper map_;
    // [TODO]: util::Storage
    ::std::optional<Input> input_;
    ::std::optional<State> state_;

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = default;
    void operator= (Continuation &&) = delete;

   public:
    Continuation(Consumer &&c, Mapper &&m)
        : cons_(::std::forward<Consumer>(c))
        , map_(::std::move(m)) {}

    void Continue(Input &&val, State st) && noexcept {
      if (CancelSource().CancelRequested()) [[unlikely]] {
        return ::std::move(cons_).CancelAware().Cancel(st);
      }

      input_.emplace(::std::move(val));
      state_.emplace(st);
      st.sched->Submit(this);
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
    void Run() && noexcept override {
      try {
        ::std::move(cons_).Continue(future::core::AdaptCall(
                                                      auto(::std::move(map_)),
                                                      *::std::move(input_)),
                                    *state_);
      } catch (...) {
        ::std::move(cons_).FailureAware().Failure(CurrentError(), *state_);
      }
    }
  };

  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return ::std::move(prod_).Materialize(Continuation<Consumer>(
                                              ::std::forward<Consumer>(c),
                                              ::std::move(map_)));
  }
};

} // namespace thunk

} // namespace exe::future::lazy
