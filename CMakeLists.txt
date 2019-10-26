#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.0)
project(Boost.UT CXX)
set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Weverything
                      -Werror
                      -pedantic
                      -pedantic-errors
                      -Wno-c++98-compat
                      -Wno-c++98-compat-pedantic
                      -Wno-c99-extensions
                      -Wno-global-constructors
                      -Wno-exit-time-destructors
                      -Wno-missing-variable-declarations
                      -Wno-class-varargs
                      -Wno-padded
                      -Wno-sign-conversion
                      -Wno-ctad-maybe-unsupported
)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall
                      -Wextra
                      -Werror
                      -pedantic
                      -pedantic-errors)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  add_compile_options(/W4
                      /Wall
                      /WX)
endif()

add_custom_target(style)
add_custom_command(TARGET style COMMAND find ${CMAKE_CURRENT_LIST_DIR}/benchmark ${CMAKE_CURRENT_LIST_DIR}/example
  ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/test -iname
  "*.hpp" -or -iname "*.cpp" | xargs clang-format -i)

option(ENABLE_MEMCHECK "Run the unit tests and examples under valgrind if it is found." OFF)
option(ENABLE_COVERAGE "Run coverage." OFF)
option(ENABLE_SANITIZERS "Run static analysis." OFF)

if (ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

if (ENABLE_SANITIZERS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")
endif()

enable_testing()
include_directories(include)
add_subdirectory(benchmark)
add_subdirectory(example)
add_subdirectory(test)
