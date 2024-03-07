// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_DEFER_HPP_INCLUDED_
#define DDVAMP_UTILS_DEFER_HPP_INCLUDED_ 1

#include <utility>

#include "macro.hpp"
#include "type_traits.hpp"

namespace utils {

template <typename T>
inline constexpr bool is_suitable_for_deferred_action_v = is_all_of_v<
    ::std::is_class_v<T>,
    ::std::is_nothrow_move_constructible_v<T>,
    ::std::is_nothrow_destructible_v<T>,
    ::std::is_invocable_v<T &> && ::std::is_void_v<::std::invoke_result_t<T &>>
>;

// RAII class for performing an action at the end of a scope
template <typename T>
requires (is_suitable_for_deferred_action_v<T>)
class [[nodiscard]] defer final {
private:
    UTILS_NO_UNIQUE_ADDRESS T action_;

public:
    constexpr ~defer() noexcept (::std::is_nothrow_invocable_v<T &>) {
        action_();
    }

    defer(defer const &) = delete;
    void operator= (defer const &) = delete;

    defer(defer &&) = delete;
    void operator= (defer &&) = delete;

public:
    constexpr explicit defer(T act) noexcept
        : action_(::std::move(act)) {}
};

} // namespace utils

#endif /* DDVAMP_UTILS_DEFER_HPP_INCLUDED_ */
