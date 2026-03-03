#include "cancel.hpp"
#include "consumer.hpp"
#include "box.hpp"

namespace exe::future::lazy {

namespace thunk {

template <typename ...Producers>
class [[nodiscard]] First final
    : public core::IBoxedThunk<trait::ValueOf<Producers...[0]>>,
      private cancel::CancelSource,
      private cancel::HandlerBase {
 private:
  using ValueType = trait::ValueOf<Producers...[0]>;

  core::IConsumer<ValueType> *cons_;
  Scheduler *sched_;
  ::std::tuple<Producers...> prod_thunks_;
  ::std::tuple<::std::optional<trait::Materialize<Producers,
                                                  First &>>...> prods_;

  ::std::atomic_bool first_ = true;
  ::std::atomic_size_t live_ = sizeof...(Producers);
  ::std::atomic_size_t ref_cnt_ = 1 + sizeof...(Producers);

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);
  static_assert(::std::atomic<cancel::HandlerBase *>::is_always_lock_free);

 public:
  First(Producers &&...prods) : prod_thunks_(::std::move(prods)...) {}

  /* IBoxedThunk */

  void Start(core::IConsumer<ValueType> &cons,
             Scheduler &sched) && noexcept final {
    cons_ = &cons;
    sched_ = &sched;

    cons_->CancelSource().AddHandler(*this);

    auto &[...prod_thunk] = prod_thunks_;
    auto &[...prod] = prods_;
    (..., prod.emplace(::std::move(prod_thunk).Materialize(*this)));
    (..., (*::std::move(prod)).Start(sched));
  }

  void Drop() && noexcept final {
    DestroySelf();
  }

  /* Continuation */

  void Continue(ValueType &&val, State st) && noexcept {
    if (TryEarly()) [[unlikely]] {
      RequestCancel();
      ::std::move(*cons_).Continue(::std::move(val), State(sched_));
    }

    Release();
  }

  void Failure(Error &&err, State st) && noexcept {
    if (TryLate()) [[unlikely]] {
      RequestCancel();
      ::std::move(*cons_).FailureAware().Failure(::std::move(err),
                                                 State(sched_));
    }

    Release();
  }

  void Cancel(State st) && noexcept {
    UTIL_ASSERT(CancelRequested(), "Upstream Cancel without request");

    if (TryEarly()) [[unlikely]] {
      ::std::move(*cons_).CancelAware().Cancel(State(sched_));
    }

    Release();
  }

  /**/

  First &&FailureAware() && noexcept {
    return ::std::move(*this);
  }

  First &&CancelAware() && noexcept {
    return ::std::move(*this);
  }

  CancelSource &CancelSource() noexcept {
    return *this;
  }

 private:
  // cancel::IHandler
  void OnCancelRequested() && noexcept override {
    RequestCancel();
    Release();
  }

  [[nodiscard]] bool TryEarly() noexcept {
    return first_.exchange(false, ::std::memory_order_relaxed);
  }

  [[nodiscard]] bool TryLate() noexcept {
    return live_.fetch_sub(1, ::std::memory_order_relaxed) == 1;
  }

  void Release() noexcept {
    if (ref_cnt_.fetch_sub(1, ::std::memory_order_release) == 1) {
      ::util::sync_with_release_sequences(ref_cnt_);
      DestroySelf();
    }
  }

  void DestroySelf() noexcept {
    delete this;
  }
};

} // namespace thunk

template <::util::rvalue_deduced ...Producers>
requires ((sizeof...(Producers) > 1) &&
          ::util::is_all_same_v<trait::ValueOf<Producers>...>)
auto First(Producers &&...prods) {
  return thunk::Box(::new thunk::First(::std::move(prods)...));
}

} // namespace exe::future::lazy
