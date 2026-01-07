//
// operator.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CONCEPT_OPERATOR_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CONCEPT_OPERATOR_HPP_INCLUDED_ 1

#include <exe/future/fun/operator/operator_fwd.hpp>

#include <concepts>
#include <utility>

namespace exe::future::concepts {

template <typename Op, typename T>
concept OperatorFor =
    ::std::destructible<Op> &&
    ::std::derived_from<Op, Operator> &&
    requires (Op op, T t) {
      { ::std::move(op).Apply(::std::move(t)) };
    };

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_FUN_CONCEPT_OPERATOR_HPP_INCLUDED_ */
