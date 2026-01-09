//
// first.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_FIRST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_FIRST_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future.hpp>
#include <exe/future/fun/make/contract.hpp>
#include <exe/future/fun/operator/operator.hpp>
#include <exe/future/fun/trait/value_of.hpp>
#include <exe/future/fun/type/future.hpp>
#include <exe/future/fun/type/result.hpp>
#include <exe/runtime/inline.hpp>

#include <util/type_traits.hpp>

#include <atomic>
#include <cstddef>
#include <functional>
#include <utility>

namespace exe::future {

namespace detail {

class FirstImpl : public Operator {
  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic_size_t::is_always_lock_free);

 private:
  template <typename T>
  struct State {
    Promise<T> p_;
    ::std::atomic_bool first_ = true;
    ::std::atomic_size_t live_;
    ::std::atomic_size_t ref_cnt_;

    State(Promise<T> p, ::std::size_t sz) noexcept
        : p_(::std::move(p))
        , live_(sz)
        , ref_cnt_(sz) {}

    void operator() (Result<T> &&res) noexcept {
      if (res.has_value() ? IsFirstOk() : IsLastError()) {
        ::std::move(p_).SetResult(::std::move(res));
      }

      if (ref_cnt_.fetch_sub(1, ::std::memory_order_acq_rel) == 1) [[unlikely]] {
        delete this;
      }
    }

    [[nodiscard]] bool IsFirstOk() noexcept {
      return first_.exchange(false, ::std::memory_order_relaxed);
    }

    [[nodiscard]] bool IsLastError() noexcept {
      return live_.fetch_sub(1, ::std::memory_order_relaxed) == 1;
    }
  };

 public:
  template <typename ...Fs>
  SemiFuture<ValueOf<Fs...[0]>> Apply(Fs ...fs) && {
    using T = ValueOf<Fs...[0]>;

    auto [f, p] = Contract<T>();

    auto const state = ::new State(::std::move(p), sizeof...(Fs));

    (..., SetCallback(SetScheduler(::std::move(fs), runtime::GetInline()),
                      ::std::ref(*state)));

    return ::std::move(f);
  }
};

} // namespace detail

template <concepts::Future ...Fs>
requires ((sizeof...(Fs) > 1) && ::util::is_all_same_v<ValueOf<Fs>...>)
inline SemiFuture<ValueOf<Fs...[0]>> First(Fs ...fs) {
  return detail::FirstImpl().Apply(::std::move(fs)...);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_CONCUR_FIRST_HPP_INCLUDED_ */
