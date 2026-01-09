//
// detach.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TERMINATE_DETACH_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TERMINATE_DETACH_HPP_INCLUDED_ 1

#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <utility>

namespace exe::future {

namespace pipe {

class [[nodiscard]] Detach : private core::Operator {
 public:
  template <typename T>
  void Apply(SemiFuture<T> f) && noexcept {
    return SetCallback(SetScheduler(::std::move(f), runtime::GetInline()),
                       Noop{});
  }
};

} // namespace pipe

inline auto Detach() noexcept {
  return pipe::Detach();
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TERMINATE_DETACH_HPP_INCLUDED_ */
