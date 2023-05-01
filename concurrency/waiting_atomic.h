// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_WAITING_ATOMIC_H_
#define DDV_CONCURRENCY_WAITING_ATOMIC_H_ 1

#if __has_include (<linux/futex.h>)
#	include "concurrency/os/linux/waiting_atomic.h"
#else
#	error "only LINUX support"
#endif

#endif /* DDV_CONCURRENCY_WAITING_ATOMIC_H_ */
