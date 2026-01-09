//
// recover.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_RECOVER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_RECOVER_HPP_INCLUDED_ 1

#include <exe/future/fun/core/mapper.hpp>
#include <exe/future/fun/make/contract.hpp>
#include <exe/future/fun/operator/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/error.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/future/fun/type/result.hpp>
#include <exe/runtime/inline.hpp>

#include <utility>

namespace exe::future {

namespace pipe {

template <core::concepts::Mapper Fn>
requires (::std::is_invocable_v<Fn &&, Error &&>)
class [[nodiscard]] Recover : public Operator {
 private:
  Fn fn_;

 public:
  explicit Recover(Fn &&fn) noexcept : fn_(::std::move(fn)) {}

  template <typename T>
  Future<T> Apply(Future<T> f) && {
    auto [nf, p] = Contract<T>();

    auto cb = [p = ::std::move(p), fn = ::std::move(fn_)]
              (Result<T> &&res) mutable noexcept {
      if (res.has_value()) {
        ::std::move(p).SetValue(*::std::move(res));
      } else {
        try {
          ::std::move(p).SetValue(::std::move(fn)(::std::move(res).error()));
        } catch (...) {
          ::std::move(p).SetError(CurrentError());
        }
      }
    };

    auto &where = GetScheduler(f);

    SetCallback(::std::move(f), ::std::move(cb));

    return SetScheduler(::std::move(nf), where);
  }
};

} // namespace pipe

template <typename Fn>
inline auto Recover(Fn fn) noexcept {
  return pipe::Recover(core::Mapper(::std::move(fn)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_RECOVER_HPP_INCLUDED_ */
