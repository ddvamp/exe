//
// pipe.hpp
// ~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_PIPE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_PIPE_HPP_INCLUDED_ 1

#include <exe/future2/thunk.hpp>
#include <exe/future2/thunk_data.hpp>

namespace exe::future::pipe {

// [TODO]: Better design
template <concepts::Thunk T, ::util::rvalue_deduced Combinator>
concepts::Thunk auto operator| (T &&t, Combinator &&c) {
  return ::std::move(c).Pipe(::std::move(t));
}

////////////////////////////////////////////////////////////////////////////////

template <typename ...Combinators>
struct Pipe {
  ThunkData<Combinators...> data;
};

template <typename ...Ts>
Pipe(Ts...) -> Pipe<Ts...>;

template <typename ...Ts, typename ...Us>
inline Thunk<Ts..., Us...> operator| (Thunk<Ts...> &&t, Pipe<Us...> &&p) {
  return ::std::move(t).Extend(::std::move(p).data);
}

} // namespace exe::future::pipe

#endif /* DDVAMP_EXE_FUTURE_PIPE_HPP_INCLUDED_ */
