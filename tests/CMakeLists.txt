#
# CMakeLists.txt
# ~~~~~~~~~~~~~~
#
# Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
#
# Licensed under GNU GPL-3.0-or-later.
# See file LICENSE or <https://www.gnu.org/licenses/> for details.
#

cmake_minimum_required(VERSION 4.1)

add_executable(exe_tests)
target_sources(exe_tests PRIVATE dummy_test.cpp)

target_link_libraries(exe_tests PRIVATE concurrency context exe util)

add_test(NAME exe_tests COMMAND exe_tests)

# ctest -L exe;tests
set_tests_properties(exe_tests PROPERTIES LABELS "exe;tests")
