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

project(
  exe
  VERSION       0.1.0
  DESCRIPTION   "Framework for concurrency"
  HOMEPAGE_URL  "https://github.com/ddvamp/exe.git"
  LANGUAGES     CXX ASM
)

set(
  exe_sources

  src/context/exceptions_context.cpp
  src/context/machine_context.cpp
  src/context/machine_context_sysv_elf.S

  src/exe/coroutine.cpp
  src/exe/fiber.cpp
  src/exe/handle.cpp
  src/exe/manual_loop.cpp
  src/exe/stack.cpp
  src/exe/strand.cpp
  src/exe/thread_pool.cpp

  src/util/abort.cpp
  src/util/assert.cpp
  src/util/assume.cpp
  src/util/page_allocation.cpp
  src/util/unreachable.cpp
)

add_library(exe STATIC)
target_sources(exe PRIVATE ${exe_sources})

target_include_directories(
  exe
  PUBLIC include/
  PRIVATE src/util/ src/exe/
)

target_compile_features(exe PUBLIC cxx_std_26)

enable_testing()
add_subdirectory(test)
