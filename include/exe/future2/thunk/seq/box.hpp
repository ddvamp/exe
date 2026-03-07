//
// box.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_SEQ_BOX_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_SEQ_BOX_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/core/proxy.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/type/future.hpp>

#include <util/debug/assert.hpp>

#include <utility>

namespace exe::future {

namespace core {

template <concepts::FutureValue V>
struct IBoxedThunk {
 protected:
  // Lifetime cannot be controlled via IBoxedThunk<> *
  ~IBoxedThunk() = default;

 public:
  virtual void Start(IConsumer<V> &, Scheduler &) && noexcept = 0;

  virtual void Drop() && noexcept = 0;
};

template <concepts::FutureValue V>
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

  // Move-out only
  BoxHolder(BoxHolder &&that) noexcept : box_(that.Release()) {}
  void operator= (BoxHolder &&) = delete;

 public:
  explicit BoxHolder(IBoxedThunk<V> *box) noexcept : box_(box) {}

  bool HasValue() noexcept {
    return box_;
  }

  [[nodiscard]] IBoxedThunk<V> *Release() noexcept {
    return ::std::exchange(box_, nullptr);
  }
};

} // namespace core

namespace thunk {

namespace detail {

template <concepts::Thunk Boxed>
class Box final : public core::IBoxedThunk<trait::ValueOf<Boxed>> {
 private:
  using ValueType = trait::ValueOf<Boxed>;
  using Comp = Boxed::template Computation<core::Proxy<Box>>;

  IConsumer<ValueType> *cons_;
  Comp comp_;

 public:
  explicit Box(Boxed &&b) noexcept
      : comp_(core::Proxy{*this}, ::std::move(b)) {}

  /* IBoxedThunk */

  void Start(IConsumer<ValueType> &c, Scheduler &s) && noexcept final {
    cons_ = &c;
    ::std::move(comp_).Start(s);
  }

  void Drop() && noexcept final {
    DestroySelf();
  }

  /* Continuation */

  void Continue(ValueType &&v, State s) && noexcept {
    ::std::move(*cons_).Continue(::std::move(v), s);
    DestroySelf();
  }

  void Cancel(State s) && noexcept {
    ::std::move(*cons_).Cancel(s);
    DestroySelf();
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return cons_->CancelSource();
  }

 private:
  void DestroySelf() noexcept {
    delete this;
  }
};

} // namespace detail

template <concepts::FutureValue V>
class [[nodiscard]] Box {
 private:
  core::BoxHolder<V> box_;

 public:
  ~Box() = default;

  Box(Box const &) = delete;
  void operator= (Box const &) = delete;

  // Move-out only
  Box(Box &&) = default;
  void operator= (Box &&) = delete;

 public:
  explicit Box(core::IBoxedThunk<V> *b) noexcept : box_(b) {}

  explicit Box(core::BoxHolder<V> b) noexcept : box_(::std::move(b)) {}

  template <concepts::Future<V> Boxed>
  explicit Box(Boxed &&t) : box_(::new detail::Box(::std::move(t))) {}

  using ValueType = V;

  template <concepts::Consumer<V> Consumer>
  struct MakeStep final : private IConsumer<V> {
    Consumer cons_;
    core::BoxHolder<V> box_;

    MakeStep(Consumer &&c, Box &b) noexcept
        : cons_(::std::forward<Consumer>(c))
        , box_(::std::move(b).box_) {
      UTIL_ASSERT(box_.HasValue(), "Empty BoxHolder");
    }

    void Start(Scheduler &sched) && noexcept {
      ::std::move(*box_.Release()).Start(*this, sched);
    }

    /* IConsumer */

    void Continue(V &&v, State s) && noexcept final {
      ::std::move(cons_).Continue(::std::move(v), s);
    }

    void Cancel(State s) && noexcept final {
      ::std::move(cons_).Cancel(s);
    }

    cancel::CancelSource &CancelSource() & noexcept final {
      return cons_.CancelSource();
    }
  };
};

template <concepts::Thunk T>
Box(T) -> Box<trait::ValueOf<T>>;

} // namespace thunk

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_BOX_HPP_INCLUDED_ */
