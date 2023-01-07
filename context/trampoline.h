#ifndef DDV_CONTEXT_TRAMPOLINE_H_
#define DDV_CONTEXT_TRAMPOLINE_H_ 1

namespace context {

// entry point for context
class ITrampoline {
public:
	virtual ~ITrampoline() = default;

	[[noreturn]] void run() noexcept;

private:
	[[noreturn]] virtual void doRun() noexcept = 0;
};

} // namespace context

#endif /* DDV_CONTEXT_TRAMPOLINE_H_ */
