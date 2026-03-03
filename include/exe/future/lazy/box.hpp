#pragma once

#include "consumer.hpp"
#include "sequence.hpp"

namespace exe::future::lazy {

namespace core {

template <typename V>
struct IBoxedThunk {
 protected:
  // Lifetime cannot be controlled via IBoxedThunk<> *
  ~IBoxedThunk() = default;

 public:
  virtual void Start(IConsumer<V> &, Scheduler &) && noexcept = 0;

  virtual void Drop() && noexcept = 0;
};

template <typename V>
class [[nodiscard]] BoxHolder {
 private:
  IBoxedThunk<V> *box_;

 public:
  ~BoxHolder() {
    if (box_) [[unlikely]] {
      ::std::move(*box_).Drop();
    }
  }

  BoxHolder(BoxHolder const &) = delete;
  void operator= (BoxHolder const &) = delete;

  BoxHolder(BoxHolder &&that) noexcept : box_(that.Release()) {}
  void operator= (BoxHolder &&) = delete;

 public:
  explicit BoxHolder(IBoxedThunk<V> *box) noexcept : box_(box) {}

  [[nodiscard]] IBoxedThunk<V> *Release() noexcept {
    return ::std::exchange(box_, nullptr);
  }
};

} // namespace core

namespace thunk {

namespace detail {

template <typename Boxed>
class Box final : public core::IBoxedThunk<trait::ValueOf<Boxed>> {
 private:
  using ValueType = trait::ValueOf<Boxed>;

  core::IConsumer<ValueType> *cons_;
  // [TODO]: single thunk-cont slot (like variant)
  Boxed prod_thunk_;
  ::std::optional<trait::Materialize<Boxed, Box &>> prod_;

 public:
  explicit Box(Boxed &&b) : prod_thunk_(::std::move(b)) {}

  /* IBoxedThunk */

  void Start(core::IConsumer<ValueType> &cons,
             Scheduler &sched) && noexcept final {
    cons_ = &cons;
    prod_.emplace(::std::move(prod_thunk_).Materialize(*this));
    (*::std::move(prod_)).Start(sched);
  }

  void Drop() && noexcept final {
    DestroySelf();
  }

  /* Continuation */

  void Continue(ValueType &&val, State st) && noexcept {
    ::std::move(*cons_).Continue(::std::move(val), st);
    DestroySelf();
  }

  void Failure(Error &&err, State st) && noexcept {
    ::std::move(*cons_).FailureAware().Failure(::std::move(err), st);
    DestroySelf();
  }

  void Cancel(State st) && noexcept {
    ::std::move(*cons_).CancelAware().Cancel(st);
    DestroySelf();
  }

  /**/

  auto &&FailureAware() && noexcept {
    return ::std::move(*this);
  }

  auto &&CancelAware() && noexcept {
    return ::std::move(*this);
  }

  auto &CancelSource() & noexcept {
    return cons_->CancelSource();
  }

 private:
  void DestroySelf() noexcept {
    delete this;
  }
};

} // namespace detail

template <typename Value>
class [[nodiscard]] Box {
 private:
  core::BoxHolder<Value> box_;

 public:
  ~Box() = default;

  Box(Box const &) = delete;
  void operator= (Box const &) = delete;

  Box(Box &&that) = default;
  void operator= (Box &&) = delete;

 public:
  using ValueType = Value;

  explicit Box(core::IBoxedThunk<Value> *b) : box_(b) {}

  explicit Box(core::BoxHolder<Value> b) : box_(::std::move(b)) {}

  // [TODO]: concept Thunk
  template <concepts::Thunk Producer>
  explicit Box(Producer t)
      : box_(::new detail::Box<Producer>(::std::move(t))) {}

  template <typename Consumer>
  class [[nodiscard]] Computation final : private core::IConsumer<Value> {
   private:
    Consumer cons_;
    core::BoxHolder<Value> box_;

   public:
    ~Computation() = default;

    Computation(Computation const &) = delete;
    void operator= (Computation const &) = delete;

    Computation(Computation &&) = default;
    void operator= (Computation &&) = delete;

   public:
    Computation(Consumer &&c, core::BoxHolder<Value> b)
        : cons_(::std::forward<Consumer>)
        , box_(::std::move(b)) {}

    void Start(Scheduler &sched) && noexcept {
      ::std::move(*box_.Release()).Start(*this, sched);
    }

    /* IConsumer */

    void Continue(Value &&val, State st) && noexcept final {
      ::std::move(cons_).Continue(::std::move(val), st);
    }

    void Failure(Error &&err, State st) && noexcept final {
      ::std::move(cons_).FailureAware().Failure(::std::move(err), st);
    }

    void Cancel(State st) && noexcept final {
      ::std::move(cons_).CancelAware().Cancel(st);
    }

    cancel::CancelSource &CancelSource() & noexcept final {
      return cons_.CancelSource();
    }
  };

  template <typename Consumer>
  auto Materialize(Consumer &&c) && noexcept {
    return Computation<Consumer>(::std::forward<Consumer>(c),
                                 ::std::move(box_));
  }
};

template <concepts::Thunk T>
Box(T) -> Box<trait::ValueOf<T>>;

} // namespace thunk

} // namespace exe::future::lazy
