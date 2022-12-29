#include <cstddef>

#include <sys/mman.h>
#include <unistd.h>

#include "utils/abort.h"

namespace utils {

namespace {

::std::size_t get_page_size() noexcept
{
	constexpr ::std::size_t kDefaultPageSize = 4096;

	auto ret = ::sysconf(_SC_PAGE_SIZE);

	return ret == -1 ? kDefaultPageSize : static_cast<::std::size_t>(ret);
}

void *allocate_memory(::std::size_t size) noexcept
{
	auto memory = ::mmap(
		NULL,
		size,
		PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		-1,
		0
	);

	if (memory == MAP_FAILED) {
		abort("mmap memory allocate failure");
	}

	return memory;
}

void protect_memory(void *address, ::std::size_t size) noexcept
{
	auto ret = ::mprotect(address, size, PROT_NONE);

	if (ret != 0) {
		abort("mprotect memory protect failure");
	}
}

void release_memory(void *address, ::std::size_t size) noexcept
{
	auto ret = ::munmap(address, size);

	if (ret != 0) {
		abort("munmap memory release failure");
	}
}

} // namespace

} // namespace utils
