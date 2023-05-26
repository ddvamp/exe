// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_
#define DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_ 1

#include "exe/futures/fun/mutator/fwd.h"
#include "exe/futures/fun/state/shared_state.h"

namespace exe::futures {

template <typename T>
class [[nodiscard]] SemiFuture : protected detail::HoldState<T> {
	template <typename>
	friend struct Contract;

	friend detail::Mutator;

protected:
	using Base = detail::HoldState<T>;

	using Base::Base;

public:
	using Base::value_type;
};

// Class for representing future value
// Future is moveable value type
template <typename T>
class [[nodiscard]] Future : public SemiFuture<T> {
	friend detail::Mutator;

protected:
	using SemiFuture<T>::SemiFuture;
};

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_TYPES_FUTURE_H_ */
