#ifndef DDV_CONTEXT_EXCEPTIONS_CONTEXT_H_
#define DDV_CONTEXT_EXCEPTIONS_CONTEXT_H_ 1

#include <cstdint>

namespace context {

class ExceptionsContext {
private:
	std::uintptr_t exceptions_state_buf_[2]{};

public:
	void switchTo(ExceptionsContext &target) noexcept;
};

} // namespace context

#endif /* DDV_CONTEXT_EXCEPTIONS_CONTEXT_H_ */