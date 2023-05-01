// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONCURRENCY_ONE_TIME_NOTIFICATION_H_
#define DDV_CONCURRENCY_ONE_TIME_NOTIFICATION_H_ 1

#if defined (__linux__)
#	include "concurrency/os/linux/one_time_notification.h"
#else
#	include "concurrency/os/general/one_time_notification.h"
#endif

#endif /* DDV_CONCURRENCY_ONE_TIME_NOTIFICATION_H_ */
