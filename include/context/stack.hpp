//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONTEXT_STACK_HPP_INCLUDED_
#define DDVAMP_CONTEXT_STACK_HPP_INCLUDED_ 1

#include <cstddef>
#include <utility>

#include "util/memory/page_allocation.hpp"

namespace context {

class Stack {
private:
	::util::page_allocation allocation_;

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
		auto allocation = ::util::page_allocation::allocate_pages(count + 1);
		allocation.protect_pages(0, 1);
		return Stack{::std::move(allocation)};
	}

	static Stack allocateBytes(::std::size_t at_least)
	{
		auto const page_size = ::util::page_allocation::page_size();

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

	::util::memory_view view() const noexcept
	{
		return allocation_.view();
	}

private:
	Stack(::util::page_allocation allocation) noexcept
		: allocation_(::std::move(allocation))
	{}
};

} // namespace context

#endif /* DDVAMP_CONTEXT_STACK_HPP_INCLUDED_ */
