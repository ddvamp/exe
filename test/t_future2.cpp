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
    int result;

    void Continue(int r, State) && noexcept {
      result = r;
    }

    void Cancel(State) && noexcept {}
    cancel::CancelSource &CancelSource() & noexcept;
  };

  Consumer cons;

  alignas (1'024) unsigned char buffer[1'024];

  Core computation(buffer,
                   cons,
                   thunk::Ready(0),
                   thunk::Via(exe::runtime::GetInline()),
                   thunk::Map([](int v) { return v + 1; }),
                   thunk::Via(exe::runtime::GetInline()),
                   thunk::Map([](int v) { return v * 2; }),
                   thunk::Via(exe::runtime::GetInline()),
                   thunk::Map([](int v) { return v + 50; }));

  ::std::move(computation).Start(exe::runtime::GetInline());

  ::std::cout << cons.result;

  return EXIT_SUCCESS;
}

} // namespace

int main() {
  return TestFuture();
}
