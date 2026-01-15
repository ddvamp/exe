//
// after.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_AFTER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_AFTER_HPP_INCLUDED_ 1

#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <chrono>
#include <utility>

namespace exe::future {

namespace pipe {

class [[nodiscard]] After : private core::Operator {
 private:
  ::std::chrono::microseconds us_;

 public:
  explicit After(::std::chrono::microseconds us) : us_(us) {}

  template <typename T>
  SemiFuture<T> Apply(SemiFuture<T> f) && {
    if (us_ == us_.zero()) {
      return ::std::move(f);
    }

    auto [nf, p] = core::Contract<T>();

    auto cb = [p = ::std::move(p), us = us_](Result<T> &&res) mutable noexcept {
      auto cb = [p = ::std::move(p), res = ::std::move(res)] mutable noexcept {
        ::std::move(p).SetResult(::std::move(res));
      };

      // [TODO]: Not implemented (send to timer)
      ::std::move(cb)();
    };

    SetCallback(SetScheduler(::std::move(f), runtime::GetInline()),
                ::std::move(cb));

    return ::std::move(nf);
  }

  template <typename T>
  Future<T> Apply(Future<T> f) && {
    auto &where = GetScheduler(f);
    return SetScheduler(::std::move(*this)
                            .Apply(static_cast<SemiFuture<T> &&>(f)), where);
  }
};

} // namespace pipe

inline auto After(::std::chrono::microseconds us) {
  return pipe::After(us);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_AFTER_HPP_INCLUDED_ */
