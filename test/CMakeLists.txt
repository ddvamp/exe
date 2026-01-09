#
# CMakeLists.txt
# ~~~~~~~~~~~~~~
#
# Copyright (C) 2025-2026 Artyom Kolpakov <ddvamp007@gmail.com>
#
# Licensed under GNU GPL-3.0-or-later.
# See file LICENSE or <https://www.gnu.org/licenses/> for details.
#

cmake_minimum_required(VERSION 4.1)

function(add_test_user test_name)
  set(test test_${test_name})

  add_executable(${test})
  target_sources(${test} PRIVATE t_${test_name}.cpp)

  target_link_libraries(${test} PRIVATE exe)

  add_test(NAME ${test} COMMAND ${test})

  # ctest -L exe/tests/[test_name]
  set_tests_properties(${test} PROPERTIES LABELS "exe;tests;${test}")
endfunction()

set(
  tests

  channel
  future
  reactor
)

foreach(test ${tests})
  add_test_user(${test})
endforeach()
