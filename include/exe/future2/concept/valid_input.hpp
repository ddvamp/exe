//
// valid_input.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CONCEPT_VALID_INPUT_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CONCEPT_VALID_INPUT_HPP_INCLUDED_ 1

namespace exe::future::concepts {

template <typename InputType, typename Combinator>
concept ValidInput = Combinator::template ValidInput<InputType>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_CONCEPT_VALID_INPUT_HPP_INCLUDED_ */
