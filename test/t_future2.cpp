//
// t_future2.cpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/future2/all_thunks.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/runtime/inline.hpp>

#include <cstdlib>
#include <iostream>

namespace {

template <typename From, typename To>
struct Mapper {
  To to;

  To operator() (From &&) && noexcept {
    return ::std::move(to);
  }
};

int TestFuture() {
  using namespace exe::future;

  struct Consumer {
    int result = 0;
    cancel::CancelSource source;

    void Continue(int r, State) && noexcept {
      result = r;
    }

    void Cancel(State) && noexcept {
      result = -1;
    }

    cancel::CancelSource &CancelSource() & noexcept {
      return source;
    }
  };

  Consumer cons;

  static_assert(concepts::Combinator<thunk::Via, int>);

  Thunk t(thunk::Ready(0),
          thunk::Via(exe::runtime::GetInline()),
          thunk::Map([](int v) { return v + 1; }),
          thunk::Via(exe::runtime::GetInline()),
          thunk::Map([](int v) { return v * 2; }),
          thunk::Via(exe::runtime::GetInline()),
          thunk::Map([](int v) { return v + 50; }));

  auto comp = ::std::move(t).Materialize(cons);
  ::std::move(comp).Start(exe::runtime::GetInline());

  ::std::cout << cons.result;

  return EXIT_SUCCESS;
}

int TestFuture2() {
  using namespace exe::future;

  auto &sched = exe::runtime::GetInline();

  Thunk t(thunk::Ready(0),
          thunk::Via(sched),
          thunk::Map([](int v) { return v + 1; }),
          thunk::Via(sched),
          thunk::Map([](int v) { return v * 2; }),
          thunk::Via(sched),
          thunk::Map([](int v) { return v + 50; }));

  auto t2 = ::std::move(t).Extend(thunk::Via(sched));

  auto t3 = ::std::move(t2).Extend(thunk::Map([](int v) { return v + 5; }),
                                   thunk::Map([](int) { return 0.0; }));

  detail::ThunkData d1{0};
  auto d2 = ::std::move(d1).Extend(0);
  auto d3 = ::std::move(d2).Extend(0, 0, 0);
  auto d4 = ::std::move(d3).Extend(detail::ThunkData{0.0});

  return EXIT_SUCCESS;
}

} // namespace

int main() {
  return TestFuture();
}
