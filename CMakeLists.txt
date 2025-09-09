#
#
#
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
  ./context/exceptions_context.cpp
  ./context/machine_context.cpp
  ./context/machine_context_sysv_elf.S
  ./exe/runtime/tp/thread_pool.cpp
  ./exe/runtime/strand.cpp
  ./exe/fiber/core/fiber.cpp
  ./exe/fiber/core/handle.cpp
  ./exe/fiber/core/stack.cpp
  ./util/debug/assert.cpp
  ./util/debug/assume.cpp
  ./util/debug/unreachable.cpp
  ./util/memory/page_allocation.cpp
  ./util/abort.cpp
)

add_library(exe STATIC)
target_sources(exe PRIVATE ${exe_sources})

target_include_directories(exe PUBLIC ./)

target_compile_features(exe PUBLIC cxx_std_23)

enable_testing()
add_subdirectory(tests)
