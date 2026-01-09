//
// t_future.cpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/future/fun/future.hpp>
#include <exe/runtime/inline.hpp>

#include <chrono>
#include <cstdlib>
#include <utility>

int TestFuture() {
  using namespace exe::future;
  using namespace std::chrono_literals;

  auto f = First(All(Just(), Value(0), Failure<int>(MakeError(0)),
                     Spawn(exe::runtime::GetInline(), [] {})) |
                     Inline() | Map([](auto) { return Value(0); }) | Flatten() |
                     Recover([](Error) { return 42; }),
                 Just() | Via(exe::runtime::GetInline()) |
                     FlatMap([] { return Value(42); }) | Map([](int) {}) |
                     Recover([](Error) {}) | Map([](Unit) { return 42; })) |
           After(1s);

  try {
    auto res = ::std::move(f) | Get();
    if (res == 42) {
      return EXIT_SUCCESS;
    }
  } catch (...) {}

  return EXIT_FAILURE;
}

int main() {
  return TestFuture();
}
