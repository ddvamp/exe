#ifndef DDV_CONTEXT_TRAMPOLINE_H_
#define DDV_CONTEXT_TRAMPOLINE_H_ 1

namespace context {

class ITrampoline {
public:
	virtual ~ITrampoline() = default;

	virtual void run() noexcept = 0;
};

} // namespace context

#endif /* DDV_CONTEXT_TRAMPOLINE_H_ */
