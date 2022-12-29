#ifndef DDV_CONTEXT_TRAMPOLINE_H_
#define DDV_CONTEXT_TRAMPOLINE_H_ 1

namespace context {

class ITrampoline {
public:
	virtual ~ITrampoline() = default;

	[[noreturn]] virtual void doRun() noexcept = 0;

	[[noreturn]] void run() noexcept;
};

} // namespace context

#endif /* DDV_CONTEXT_TRAMPOLINE_H_ */
