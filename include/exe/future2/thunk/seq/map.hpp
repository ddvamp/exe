//
// map.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_SEQ_MAP_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_SEQ_MAP_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>
#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/core/adapt_call.hpp>
#include <exe/future2/core/concept/adapt_call.hpp>
#include <exe/future2/core/trait/adapt_call.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk_resource.hpp>
#include <exe/runtime/task/task.hpp>

#include <optional>
#include <type_traits>
#include <utility>

namespace exe::future::thunk {

namespace detail {

template <typename Mapper, typename InputType>
using MapResult = core::trait::AdaptCallResult<Mapper, InputType>;

template <typename Mapper, typename InputType>
concept ValidInput =
    concepts::FutureValue<InputType> &&
    core::concepts::AdaptCallInvocable<Mapper, InputType> &&
    concepts::FutureValue<MapResult<Mapper, InputType>>;

} // namespace detail

template <concepts::ThunkResource Mapper>
class [[nodiscard]] Map {
 private:
  Mapper mapper_;

 public:
  ~Map() = default;

  Map(Map const &) = delete;
  void operator= (Map const &) = delete;

  // Move-out only
  Map(Map &&) = default;
  void operator= (Map &&) = delete;

 public:
  explicit Map(Mapper m) noexcept : mapper_(::std::move(m)) {}

  /* Combinator */

  template <typename InputType>
  inline static constexpr bool ValidInput =
      detail::ValidInput<Mapper, InputType>;

  template <concepts::ValidInput<Map> InputType>
  using ValueType = detail::MapResult<Mapper, InputType>;

  template <concepts::ValidInput<Map> InputType,
            concepts::Continuation<ValueType<InputType>> Consumer>
  requires (::std::is_nothrow_destructible_v<Consumer>)
  class [[nodiscard]] Continuation : private Consumer {
   private:
    struct Task final : runtime::task::TaskBase {
      Continuation *cont;

      Task(Continuation *c) : cont(c) {}

      void Run() && noexcept final {
        cont->RunImpl();
      }
    };

    Task task{this};
    Mapper mapper_;
    ::std::optional<::std::pair<InputType, State>> val_; // [TODO]: ?struct

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = delete;
    void operator= (Continuation &&) = delete;

   public:
    template <typename ...Args>
    requires (::std::is_nothrow_constructible_v<Consumer, Args...>)
    explicit Continuation(Map &&m, Args &&...args) noexcept
        : Consumer(::std::forward<Args>(args)...)
        , mapper_(::std::move(m).mapper_) {}

    /* Continuation */

    void Continue(InputType &&v, State s) && noexcept {
      if (!TryCancel(s)) [[likely]] {
        val_.emplace(::std::move(v), s);
        s.sched.Submit(&task);
      }
    }

    using Consumer::Cancel;
    using Consumer::CancelSource;

   private:
    void RunImpl() noexcept {
      auto &[v, s] = *val_;
      if (TryCancel(s)) [[unlikely]] {
        return;
      }

      try {
        static_cast<Consumer &&>(*this).Continue(
            core::AdaptCall(auto(::std::move(mapper_)),
                            ::std::move(v)),
            s);
      } catch (...) {
        ::std::terminate(); // [TODO]: log
      }
    }

    bool TryCancel(State s) {
      // if (this->CancelSource().CancelRequested()) [[unlikely]] {
      //   ::std::move(*this).Cancel(s);
      //   return true;
      // }

      return false;
    }
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_MAP_HPP_INCLUDED_ */
