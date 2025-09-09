// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONTEXT_EXCEPTIONS_CONTEXT_H_
#define DDV_CONTEXT_EXCEPTIONS_CONTEXT_H_ 1

#include <cstdint>

namespace context {

class ExceptionsContext {
private:
	::std::uintptr_t exceptions_state_buf_[2]{};

public:
	// save current context in this and reset target context
	// (this and target are allowed to be aliased)
	void switchTo(ExceptionsContext &target) noexcept;
};

} // namespace context

#endif /* DDV_CONTEXT_EXCEPTIONS_CONTEXT_H_ */
