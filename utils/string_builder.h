#ifndef DDV_UTILS_STRING_BUILDER_H_
#define DDV_UTILS_STRING_BUILDER_H_

#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace utils {

inline ::std::string reservedString(::std::size_t capacity)
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

class StringBuilder {
private:
	::std::ostringstream os_;

public:
	StringBuilder() = default;

	explicit StringBuilder(::std::size_t capacity)
		: os_(reservedString(capacity))
	{}
	

	explicit StringBuilder(::std::string init)
		: os_(::std::move(init))
	{}

	StringBuilder &operator<< (auto const &next)
	{
		os_ << next;
		return *this;
	}

	StringBuilder &operator<< (::std::ios_base &(*manip)(::std::ios_base &))
	{
		os_ << manip;
		return *this;
	}

	StringBuilder &operator<< (::std::ios &(*manip)(::std::ios &))
	{
		os_ << manip;
		return *this;
	}

	StringBuilder &operator<< (::std::ostream &(*manip)(::std::ostream &))
	{
		os_ << manip;
		return *this;
	}

	StringBuilder &&operator<< (move_t) noexcept
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

} // namespace utils

#endif /* DDV_UTILS_STRING_BUILDER_H_ */
