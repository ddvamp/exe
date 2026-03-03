//
// t_future_lazy.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/future/lazy/first.hpp>
#include <exe/runtime/inline.hpp>

#include <cstdlib>

template <typename V>
exe::future::lazy::core::IConsumer<V> &Cons();

int TestFutureLazy() {
  using namespace exe::future::lazy;
  using namespace exe::future::lazy::thunk;
  using exe::future::lazy::Ready;
  using exe::future::lazy::Sequence;
  using exe::future::lazy::First;

  auto r0 = Ready(0);
  auto r1 = Ready(1);
  auto r2 = Ready(2);

  auto t0 = Via(::std::move(r0), exe::runtime::GetInline());
  auto t1 = Map(::std::move(r1), [](int i) { return i * 2; }).Materialize(Cons<int>());
  // auto t2 = Map(::std::move(r2), [](int i) { return Ready(i); });
  // auto t3 = Flatten(::std::move(t2));
  // auto t4 = Sequence(::std::move(t1), ::std::move(t3));

  // auto f = First(::std::move(t0), ::std::move(t4));

  return EXIT_SUCCESS;
}

int main() {
  return TestFutureLazy();
}
