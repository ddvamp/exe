//
// future.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_TYPE_FUTURE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_TYPE_FUTURE_HPP_INCLUDED_ 1

#include <exe/future2/model/thunk.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <concepts>

namespace exe::future::concepts {

template <typename F>
concept SomeFuture = Thunk<F>;

template <typename F, typename V>
concept Future = SomeFuture<F> && ::std::same_as<trait::ValueOf<F>, V>;

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_TYPE_FUTURE_HPP_INCLUDED_ */
