//
// consumer.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MODEL_CONSUMER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MODEL_CONSUMER_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>

namespace exe::future {

template <concepts::FutureValue InputType>
struct IConsumer {
 protected:
  // Lifetime cannot be controlled via IConsumer<> *
  ~IConsumer() = default;

 public:
  virtual void Continue(InputType &&, State) && noexcept = 0;

  virtual void Cancel(State) && noexcept = 0;

  virtual cancel::CancelSource &CancelSource() & noexcept = 0;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MODEL_CONSUMER_HPP_INCLUDED_ */
