// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONTEXT_STACK_H_
#define DDV_CONTEXT_STACK_H_ 1

#include <cstddef>
#include <utility>

#include "utils/memory/page_allocation.h"

namespace context {

class Stack {
private:
	::utils::page_allocation allocation_;

public:
	~Stack() = default;

	Stack(Stack const &) = delete;
	void operator= (Stack const &) = delete;

	Stack(Stack &&) noexcept = default;
	Stack &operator= (Stack &&) noexcept = default;

public:
	Stack() = default;

	static Stack allocatePages(::std::size_t count)
	{
		auto allocation = ::utils::page_allocation::allocate_pages(count + 1);
		allocation.protect_pages(0, 1);
		return Stack{::std::move(allocation)};
	}

	static Stack allocateBytes(::std::size_t at_least)
	{
		auto const page_size = ::utils::page_allocation::page_size();

		auto pages = at_least / page_size;
		if (at_least % page_size != 0) {
			++pages;
		}

		return allocatePages(pages);
	}

	::std::size_t allocationSize() const noexcept
	{
		return allocation_.size();
	}

	::utils::memory_view view() const noexcept
	{
		return allocation_.view();
	}

private:
	Stack(::utils::page_allocation allocation) noexcept
		: allocation_(::std::move(allocation))
	{}
};

} // namespace context

#endif /* DDV_CONTEXT_STACK_H_ */
