# Copyright (C) 2025 Artyom Kolpakov <ddvamp007@gmail.com>
# Licensed under GNU GPL-3.0-or-later.
# See file LICENSE or <https://www.gnu.org/licenses/> for details.

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
  ./context/arch/x86_64/machine_context_sysv_elf.S
  ./context/arch/x86_64/machine_context.cc
  ./context/exceptions_context.cc
  ./exe/coroutine/coroutine.cc
  ./exe/executors/tp/thread_pool.cc
  ./exe/executors/strand.cc
  ./exe/fibers/core/fiber.cc
  ./exe/fibers/core/handle.cc
  ./exe/fibers/core/stack.cc
  ./utils/debug/assert.cc
  ./utils/debug/assume.cc
  ./utils/debug/unreachable.cc
  ./utils/memory/page_allocation.cc
  ./utils/abort.cc
)

add_library(exe STATIC)
target_sources(exe PRIVATE ${exe_sources})

target_include_directories(exe PUBLIC ./)

target_compile_features(exe PUBLIC cxx_std_23)

enable_testing()
add_subdirectory(tests)
