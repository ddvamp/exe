#
# CMakeLists.txt
# ~~~~~~~~~~~~~~~~~~~
#
# Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
#
# Licensed under GNU GPL-3.0-or-later.
# See file LICENSE or <https://www.gnu.org/licenses/> for details.
#

cmake_minimum_required(VERSION 4.1)

project(
  exe
  VERSION       0.1.0
  DESCRIPTION   "Framework for concurrency"
  HOMEPAGE_URL  "https://github.com/ddvamp/exe.git"
  LANGUAGES     CXX ASM
)

set(
  exe_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/src/coroutine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fiber.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/handle.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/inline.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/run_loop.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/stack.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/strand.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/thread_pool.cpp
)

add_library(exe STATIC)
target_sources(exe PRIVATE ${exe_sources})

target_include_directories(
  exe
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(exe PUBLIC cxx_std_23)

add_subdirectory(third_party)
target_link_libraries(exe PUBLIC concurrency context util)

enable_testing()
add_subdirectory(tests)
