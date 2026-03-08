//
// t_future2.cpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/future2/cancel.hpp>
#include <exe/future2/all_thunks.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/runtime/inline.hpp>

#include <util/abort.hpp>
#include <util/debug.hpp>

#include <cstdlib>
#include <iostream>
#include <utility>

namespace exe::future {

namespace {

struct AnyConsumer {
  int success = -1;
  cancel::CancelSource source;

  void Continue(auto &&, State) && noexcept {
    success = 1;
  }

  void Cancel(State) && noexcept {
    success = 0;
  }

  auto &CancelSource() & noexcept {
    return source;
  }
};

template <concepts::FutureValue V>
struct Consumer {
  ::std::optional<V> res;
  cancel::CancelSource source;

  void Continue(V &&v, State) && noexcept {
    res.emplace(::std::move(v));
    source.RequestCancel();
  }

  void Cancel(State) && noexcept {
    UTIL_ABORT("Unexpected upstream Cancel() call");
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return source;
  }
};

} // namespace

} // namespace exe::future

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

  ThunkData d1{0};
  auto d2 = ::std::move(d1).Extend(0);
  auto d3 = ::std::move(d2).Extend(0, 0, 0);
  auto d4 = ::std::move(d3).Extend(ThunkData{0.0});

  return EXIT_SUCCESS;
}

int TestSequence() {
  using namespace exe::future;

  auto t = Sequence(Thunk(thunk::Ready(0)),
                    Thunk(thunk::Ready(0.0)),
                    Thunk(thunk::Ready(nullptr)));

  AnyConsumer cons;
  auto comp = ::std::move(t).Materialize(cons);
  ::std::move(comp).Start(exe::runtime::GetInline());

  return (cons.success == 1 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int TestFlatten() {
  using namespace exe::future;

  Thunk t(thunk::Ready(Thunk(thunk::Ready(42))),
          thunk::Flatten{});

  Consumer<int> cons;
  auto comp = ::std::move(t).Materialize(cons);
  ::std::move(comp).Start(exe::runtime::GetInline());

  return (cons.res.value_or(0) == 42 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int TestBox() {
  using namespace exe::future;

  Thunk t(thunk::Box(Thunk(thunk::Ready(42))));

  Consumer<int> cons;
  auto comp = ::std::move(t).Materialize(cons);
  ::std::move(comp).Start(exe::runtime::GetInline());

  return (cons.res.value_or(0) == 42 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int TestFirst() {
  using namespace exe::future;

  Consumer<int> cons;

  auto t = First(Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)));

  auto comp = ::std::move(t).Materialize(cons);
  ::std::move(comp).Start(exe::runtime::GetInline());

  return (cons.res.value_or(1) == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int TestAll() {
  using namespace exe::future;

  auto t = All(Thunk(thunk::Ready(0)),
               Thunk(thunk::Ready(1)),
               Thunk(thunk::Ready(2)),
               Thunk(thunk::Ready(3)),
               Thunk(thunk::Ready(4)),
               Thunk(thunk::Ready(5)));

  using R = trait::ValueOf<decltype(t)>;

  Consumer<R> cons;
  auto comp = ::std::move(t).Materialize(cons);
  ::std::move(comp).Start(exe::runtime::GetInline());

  if (!cons.res.has_value()) {
    return EXIT_FAILURE;
  }

  auto success = [&cons]<::std::size_t ...Is>(::std::index_sequence<Is...>) {
    auto &[...val] = *cons.res;
    return (... && (val == Is));
  }(::std::make_index_sequence<6>{});

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace

#define RUN_TEST(test) UTIL_CHECK(test() == EXIT_SUCCESS, #test);

int main() {
  RUN_TEST(TestFuture);
  RUN_TEST(TestFuture2);
  RUN_TEST(TestSequence);
  RUN_TEST(TestFlatten);
  RUN_TEST(TestBox);
  RUN_TEST(TestFirst);
  RUN_TEST(TestAll);

  return EXIT_SUCCESS;
}
