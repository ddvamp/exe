#ifndef DDV_UTILS_MEMORY_VIEW_H_
#define DDV_UTILS_MEMORY_VIEW_H_ 1

#include <span>

namespace utils {

using memory_view = ::std::span<::std::byte>;

}

#endif /* DDV_UTILS_MEMORY_VIEW_H_ */
