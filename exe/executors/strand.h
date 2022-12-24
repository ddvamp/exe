#ifndef DDV_EXE_EXECUTORS_STRAND_H_
#define DDV_EXE_EXECUTORS_STRAND_H_ 1

#if defined(__x86_64__) || defined(_M_X64)
#	include "exe/executors/arch/x86_64/strand.h"
#else
#	error "unsupported environment (not x86_64 architecture)"
#endif

#endif /* DDV_EXE_EXECUTORS_STRAND_H_ */
