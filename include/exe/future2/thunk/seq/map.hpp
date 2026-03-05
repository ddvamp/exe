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

  template <typename InputType>
  inline static constexpr bool ValidInput =
      detail::ValidInput<Mapper, InputType>;

  template <concepts::ValidInput<Map> InputType>
  using ValueType = detail::MapResult<Mapper, InputType>;

  template <concepts::ValidInput<Map> InputType,
            concepts::Continuation<ValueType<InputType>> Consumer>
  struct CombineStep final : private runtime::task::TaskBase {
    Consumer cons_;
    Map &data_;

    // [TODO]: ?::util::storage
    ::std::optional<::std::pair<InputType, State>> input_; // [TODO]: ?struct

    CombineStep(Consumer &&c, Map &m)
        : cons_(::std::forward<Consumer>(c))
        , data_(m) {}

    void Continue(InputType &&v, State s) && noexcept {
      if (TryCancel(s)) [[unlikely]] {
        return;
      }

      input_.emplace(::std::move(v), s);
      s.sched.Submit(this);
    }

    void Cancel(State s) && noexcept {
      ::std::move(cons_).Cancel(s);
    }

   private:
    // TaskBase
    void Run() && noexcept final {
      auto &[v, s] = *input_;
      if (TryCancel(s)) [[unlikely]] {
        return;
      }

      try {
        ::std::move(cons_).Continue(
            core::AdaptCall(auto(::std::move(data_).mapper_),
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
