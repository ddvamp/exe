//
// release_ptr.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CORE_RELEASE_PTR_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CORE_RELEASE_PTR_HPP_INCLUDED_ 1

#include <exe/future2/core/ptr.hpp>

#include <util/debug/assert.hpp>

#include <string_view>

namespace exe::future::core {

// [TODO]: ?Move to util
template <typename T>
class [[nodiscard]] ReleasePtr : protected Ptr<T> {
 private:
  using Base = Ptr<T>;

 public:
  constexpr ~ReleasePtr() {
    // [TODO]: Make util debug marcos constexpr
    constexpr ::std::string_view msg
        = "core::ReleasePtr is not used at the time of destruction";
    if consteval {
      if (IsValid()) {
        CompileError(msg);
      }
    } else {
      UTIL_ASSERT(!IsValid(), msg);
    }
  }

  ReleasePtr(ReleasePtr const &) = delete;
  void operator= (ReleasePtr const &) = delete;

  // Move-out only
  constexpr ReleasePtr(ReleasePtr &&that) noexcept : Base(that.Release()) {}
  void operator= (ReleasePtr &&) = delete;

 public:
  constexpr static ReleasePtr Invalid() noexcept {
    return ReleasePtr(nullptr);
  }

  using Base::Base;

  using Base::IsValid;
  using Base::Get;
  using Base::GetChecked;
  using Base::Release;
  using Base::ReleaseChecked;
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_CORE_RELEASE_PTR_HPP_INCLUDED_ */
