// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_MUTATOR_FWD_H_
#define DDV_EXE_FUTURE_FUN_MUTATOR_FWD_H_ 1

#include <concepts>

namespace exe::future {

namespace detail {

class Mutator;

} // namespace detail

namespace concepts {

template <typename M>
concept Mutator =
	::std::destructible<M> &&
	::std::derived_from<M, detail::Mutator>;

} // namespace concepts

} // namespace exe::future

#endif /* DDV_EXE_FUTURE_FUN_MUTATOR_FWD_H_ */
