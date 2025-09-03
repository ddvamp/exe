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

include(FetchContent)

FetchContent_Declare(
  concurrency
  GIT_REPOSITORY  https://github.com/ddvamp/concurrency.git
  GIT_TAG         origin/main
  GIT_SHALLOW     ON # Только указанный тэг, без всей истории ветки
  GIT_PROGRESS    OFF # Полезно включать для больших зависимостей
  # SOURCE_DIR
  # BINARY_DIR
)

FetchContent_Declare(
  context
  GIT_REPOSITORY  https://github.com/ddvamp/context.git
  GIT_TAG         origin/main
  GIT_SHALLOW     ON
  GIT_PROGRESS    OFF
)

FetchContent_Declare(
  util
  GIT_REPOSITORY  https://github.com/ddvamp/util.git
  GIT_TAG         origin/main
  GIT_SHALLOW     ON
  GIT_PROGRESS    OFF
)

# Должен быть расположен после всех FetchContent_Declare
FetchContent_MakeAvailable(
  concurrency
  context
  util
)
