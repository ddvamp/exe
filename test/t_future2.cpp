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
#include <util/macro.hpp>

#include <cstdlib>
#include <utility>

namespace {

template <typename From, typename To>
struct Mapper {
  To to;

  To operator() (From &&) && noexcept {
    return ::std::move(to);
  }
};

int TestUnify() {
  using namespace exe::future;

  {
    auto v = core::UnifyReturnInvoke([] {});
    static_assert(::std::is_same_v<exe::Unit, decltype(v)>);
  }

  {
    auto v = core::UnifyReturnInvoke([](int) {}, 0);
    static_assert(::std::is_same_v<exe::Unit, decltype(v)>);
  }

  {
    core::UnifyArgInvoke([](exe::Unit) {}, exe::Unit{});
  }

  {
    core::UnifyArgInvoke([] {}, exe::Unit{});
  }

  {
    auto v = core::UnifyInvoke([] {});
    static_assert(::std::is_same_v<exe::Unit, decltype(v)>);
  }

  {
    auto v = core::UnifyInvoke([](int) {}, 0);
    static_assert(::std::is_same_v<exe::Unit, decltype(v)>);
  }

  {
    auto v = core::UnifyInvoke([](exe::Unit) { return 0; }, exe::Unit{});
    static_assert(::std::is_same_v<int, decltype(v)>);
  }

  {
    auto v = core::UnifyInvoke([] { return 0; }, exe::Unit{});
    static_assert(::std::is_same_v<int, decltype(v)>);
  }

  {
    auto v = core::UnifyInvoke([](exe::Unit) {}, exe::Unit{});
    static_assert(::std::is_same_v<exe::Unit, decltype(v)>);
  }

  {
    auto v = core::UnifyInvoke([] {}, exe::Unit{});
    static_assert(::std::is_same_v<exe::Unit, decltype(v)>);
  }

  return EXIT_SUCCESS;
}

int TestFuture() {
  using namespace exe::future;

  static_assert(concepts::Combinator<thunk::Via, int>);

  Thunk t(thunk::Ready(0),
          thunk::Via(exe::runtime::GetInline()),
          thunk::Map([](int v) { return v + 1; }),
          thunk::Via(exe::runtime::GetInline()),
          thunk::Map([](int v) { return v * 2; }),
          thunk::Via(exe::runtime::GetInline()),
          thunk::Map([](int v) { return v + 50; }));

  auto res = ::std::move(t) | Get();
  return res == 52? EXIT_SUCCESS : EXIT_FAILURE;
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
                    Thunk(thunk::Ready<void *>(nullptr)));

  auto res = ::std::move(t) | Get();
  return res == nullptr ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestFlatten() {
  using namespace exe::future;

  Thunk t(thunk::Ready(Thunk(thunk::Ready(42))),
          thunk::Flatten{});

  auto res = ::std::move(t) | Get();
  return res == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestBox() {
  using namespace exe::future;

  Thunk t(thunk::Box(Thunk(thunk::Ready(42))));

  auto res = ::std::move(t) | Get();
  return res == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestFirst() {
  using namespace exe::future;

  auto t = First(Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)),
                 Thunk(thunk::Ready(0)));

  auto res = ::std::move(t) | Get();
  return res == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestAll() {
  using namespace exe::future;

  auto t = All(Thunk(thunk::Ready(0)),
               Thunk(thunk::Ready(1)),
               Thunk(thunk::Ready(2)),
               Thunk(thunk::Ready(3)),
               Thunk(thunk::Ready(4)),
               Thunk(thunk::Ready(5)));

  auto res = ::std::move(t) | Get();

  auto success = [&res]<::std::size_t ...Is>(::std::index_sequence<Is...>) {
    auto &[...val] = res;
    return (... && (val == Is));
  }(::std::make_index_sequence<6>{});

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestReady() {
  using namespace exe::future;

  auto t = Ready(42);

  auto res = ::std::move(t) | Get();
  return res == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestValue() {
  using namespace exe::future;

  auto t = Value(42);

  auto res = ::std::move(t) | Get();
  return res == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestJust() {
  using namespace exe::future;

  auto t = Just();

  auto res = ::std::move(t) | Get();
  static_assert(::std::is_same_v<exe::Unit, decltype(res)>);
  return EXIT_SUCCESS;
}

int TestSpawn() {
  using namespace exe::future;

  auto t = Spawn(exe::runtime::GetInline(), []{ return 42; });

  auto res = ::std::move(t) | Get();
  return res == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestPipe() {
  using namespace exe::future;

  auto &sched = exe::runtime::GetInline();

  auto t1 = Just() | Via(sched) | Map([] { return 42; });
  auto t2 = Sequence(Just(), Ready(0), Ready(42));
  auto t3 = Just() | FlatMap([&] { return Ready(42) | Via(sched); });
  auto t4 = First(::std::move(t1), ::std::move(t2), ::std::move(t3));
  auto t5 = All(::std::move(t4), Just());

  auto res = ::std::move(t5) | Get();
  return ::std::get<0>(res) == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestStart() {
  using namespace exe::future;

  auto &s = exe::runtime::GetInline();

  Thunk t(thunk::Ready(1),
          thunk::Via(s),
          thunk::Map([](int i) { return i * 5; }));

  auto eager = ::std::move(t) | Start(s);

  auto res = ::std::move(t) | Get();
  return res == 5 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestContract() {
  using namespace exe::future;

  auto [f, p] = Contract<int>();

  ::std::move(p).Set(42);

  auto res = ::std::move(f) | Get();
  return res == 42 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int TestSafeContract() {
  using namespace exe::future;

  auto [f, p] = SafeContract<int>();

  UTIL_IGNORE(auto(::std::move(p)));

  auto res = ::std::move(f) | Get();
  return !res.has_value() ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace

#define RUN_TEST(test) UTIL_CHECK(test() == EXIT_SUCCESS, #test);

int main() {
  RUN_TEST(TestUnify);
  RUN_TEST(TestFuture);
  RUN_TEST(TestFuture2);
  RUN_TEST(TestSequence);
  RUN_TEST(TestFlatten);
  RUN_TEST(TestBox);
  RUN_TEST(TestFirst);
  RUN_TEST(TestAll);
  RUN_TEST(TestReady);
  RUN_TEST(TestValue);
  RUN_TEST(TestJust);
  RUN_TEST(TestSpawn);
  RUN_TEST(TestPipe);
  RUN_TEST(TestStart);
  RUN_TEST(TestContract);
  RUN_TEST(TestSafeContract);

  return EXIT_SUCCESS;
}
