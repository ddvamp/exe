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

#include <util/debug.hpp>

#include <string_view>
#include <utility>

namespace exe::future::core {

// [TODO]: ?
[[noreturn]] consteval void CompileError(::std::string_view msg) noexcept {
  throw msg;
}

// [TODO]: ?Move to util
template <typename T>
class [[nodiscard]] ReleasePtr {
 private:
  T *ptr_;

 public:
  constexpr ~ReleasePtr() {
    // [TODO]: Make util debug marcos constexpr
    constexpr ::std::string_view msg
        = "ReleasePtr is not used at the time of destruction";
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
  constexpr ReleasePtr(ReleasePtr &&that) noexcept : ptr_(that.Release()) {}
  void operator= (ReleasePtr &&) = delete;

 public:
  constexpr explicit ReleasePtr(T *state) noexcept : ptr_(state) {}

  constexpr static ReleasePtr Invalid() noexcept {
    return ReleasePtr(nullptr);
  }

  constexpr bool IsValid() const noexcept {
    return ptr_;
  }

  /* Unchecked access */

  constexpr T *Get() noexcept {
    return ptr_;
  }

  constexpr T const *Get() const noexcept {
    return ptr_;
  }

  [[nodiscard]] constexpr T *Release() noexcept {
    return ::std::exchange(ptr_, nullptr);
  }

  /* Checked access */

  constexpr T *GetChecked() noexcept {
    Check();
    return ptr_;
  }

  constexpr T const *GetChecked() const noexcept {
    Check();
    return ptr_;
  }

  [[nodiscard]] constexpr T *ReleaseChecked() noexcept {
    Check();
    return Release();
  }

 private:
  constexpr void Check() const noexcept {
    // [TODO]: Make util debug marcos constexpr
    constexpr ::std::string_view msg
        = "ReleasePtr does not hold pointer when it is expected";
    if consteval {
      if (!IsValid()) {
        CompileError(msg);
      }
    } else {
      UTIL_ASSERT(IsValid(), msg);
    }
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_CORE_RELEASE_PTR_HPP_INCLUDED_ */
