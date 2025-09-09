// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTIL_STRING_BUILDER_H_
#define DDV_UTIL_STRING_BUILDER_H_

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace util {

inline ::std::string make_reserved_string(::std::size_t capacity)
{
	::std::string str;
	str.reserve(capacity);
	return str;
}

struct move_t {};
inline constexpr move_t move{};

struct str_t {};
inline constexpr str_t str{};

struct view_t {};
inline constexpr view_t view{};

// class that allows to fill a string with output operations to a stream
class string_builder {
private:
	::std::ostringstream os_;

public:
	string_builder() = default;

	explicit string_builder(::std::size_t capacity)
		: os_(make_reserved_string(capacity))
	{}


	explicit string_builder(::std::string init)
		: os_(::std::move(init))
	{}

	string_builder &operator<< (auto const &next)
	{
		os_ << next;
		return *this;
	}

	string_builder &operator<< (::std::ios_base &(*manip)(::std::ios_base &))
	{
		os_ << manip;
		return *this;
	}

	string_builder &operator<< (::std::ios &(*manip)(::std::ios &))
	{
		os_ << manip;
		return *this;
	}

	string_builder &operator<< (::std::ostream &(*manip)(::std::ostream &))
	{
		os_ << manip;
		return *this;
	}

	string_builder &&operator<< (move_t) noexcept
	{
		return ::std::move(*this);
	}

	::std::string operator<< (str_t) const &
	{
		return os_.str();
	}

	::std::string operator<< (str_t) &&
	{
		return ::std::move(os_).str();
	}

	::std::string_view operator<< (view_t) const noexcept
	{
		return os_.view();
	}

	::std::string str() const &
	{
		return os_.str();
	}

	::std::string str() &&
	{
		return ::std::move(os_).str();
	}

	operator ::std::string() const &
	{
		return os_.str();
	}

	operator ::std::string() &&
	{
		return ::std::move(os_).str();
	}

	::std::string_view view() const noexcept
	{
		return os_.view();
	}

	operator ::std::string_view() const noexcept
	{
		return os_.view();
	}
};

} // namespace util

#endif /* DDV_UTIL_STRING_BUILDER_H_ */
