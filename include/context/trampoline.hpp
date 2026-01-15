//
// trampoline.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONTEXT_TRAMPOLINE_HPP_INCLUDED_
#define DDVAMP_CONTEXT_TRAMPOLINE_HPP_INCLUDED_ 1

#include <util/debug/unreachable.hpp>

namespace context {

/* Entry point for context */
class ITrampoline {
 public:
  virtual ~ITrampoline() = default;

  [[noreturn]] void Run() noexcept {
    DoRun();
    UTIL_UNREACHABLE("ITrampoline::Run out of bounds");
  }

 private:
  [[noreturn]] virtual void DoRun() noexcept = 0;
};

} // namespace context

#endif /* DDVAMP_CONTEXT_TRAMPOLINE_HPP_INCLUDED_ */
