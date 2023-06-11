// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBERS_CORE_ID_H_
#define DDV_EXE_FIBERS_CORE_ID_H_ 1

#include <cstdint>

namespace exe::fibers {

using FiberId = ::std::uint64_t;

inline static constexpr auto kInvalidFiberId = FiberId(0);

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_ID_H_ */
