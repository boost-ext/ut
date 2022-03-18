#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.12...3.20)
project(boost.ut.test LANGUAGES CXX)

if(TEST_INSTALLED_VERSION)
  if(NOT DEFINED CMAKE_CXX_STANDARD)
    option(CXX_STANDARD_REQUIRED "Require c++ standard" YES)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_EXTENSIONS NO)
  endif()

  find_package(ut 1.1 REQUIRED)
  include(../cmake/AddCustomCommandOrTest.cmake)

  enable_testing()
endif()


if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if(WIN32) # clang-cl
    add_compile_options(
      -Wall
      -Wextra
      # FIXME -Werror
    )
  else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  endif()
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  # FIXME: we should not export this pedantic options! CK
  add_compile_options(
    -Wall
    -Wextra
    # TODO: why not simpply -Wpedantic
    -Werror
    -Wcast-align
    #-Wcast-align=strict
    -Wcast-qual
    -Wdouble-promotion
    -Wduplicated-branches
    -Wduplicated-cond
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wnull-dereference
    -Wold-style-cast
    -Wpointer-arith
    -Wredundant-decls
    -Wsign-conversion
    -Wswitch-enum
    -Wtrampolines
    -Wunused-but-set-variable
    -Wunused-result
    -Wuseless-cast
    -Wzero-as-null-pointer-constant
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_compile_options(/W4 /WX)
endif()

add_subdirectory(ut)
add_subdirectory(ft)
