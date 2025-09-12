//
// exceptions_context.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONTEXT_EXCEPTIONS_CONTEXT_HPP_INCLUDED_
#define DDVAMP_CONTEXT_EXCEPTIONS_CONTEXT_HPP_INCLUDED_ 1

#include <cstdint>

namespace context {

class ExceptionsContext {
 private:
  using State = ::std::uintptr_t[2];

  State state_{};

 public:
  // Save current context in this and reset target context
  // (this and target are allowed to be aliased)
  void SwitchTo(ExceptionsContext &target) noexcept;
};

} // namespace context

#endif /* DDVAMP_CONTEXT_EXCEPTIONS_CONTEXT_HPP_INCLUDED_ */
