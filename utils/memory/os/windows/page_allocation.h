#include <cstddef>

#include <Windows.h>

#include "utils/abort.h"

namespace utils {

namespace {

::std::size_t get_page_size() noexcept
{
	constexpr ::std::size_t kDefaultPageSize = 4096;

	::SYSTEM_INFO info;
	::GetSystemInfo(&info);

	return info.dwPageSize <= 0 ?
		kDefaultPageSize :
		static_cast<::std::size_t>(info.dwPageSize);
}

void *allocate_memory(::std::size_t size) noexcept
{
	auto memory = ::VirtualAlloc(
		NULL,
		size,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE
	);

	if (memory == NULL) {
		abort("VirtualAlloc memory allocate failure");
	}

	return memory;
}

void protect_memory(void *address, ::std::size_t size) noexcept
{
	::DWORD old;
	auto ret = ::VirtualProtect(address, size, PAGE_NOACCESS, &old);

	if (!ret) {
		abort("VirtualProtect memory protect failure");
	}
}

void release_memory(void *address, ::std::size_t) noexcept
{
	auto ret = ::VirtualFree(address, 0, MEM_RELEASE);

	if (!ret) {
		abort("VirtualFree memory release failure");
	}
}

} // namespace

} // namespace utils
