#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.0)
project(boost.ut CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(boost.ut INTERFACE)
target_include_directories(boost.ut INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
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
                      -Wno-missing-prototypes
                      -Wno-ctad-maybe-unsupported)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(-Wall
                      -Wextra
                      -Werror
                      -pedantic
                      -pedantic-errors
                      -Wfloat-equal
                      -Wlogical-op
                      -Wundef
                      -Wredundant-decls
                      -Wpointer-arith
                      -Wcast-qual
                      -Wcast-align
                      -Wuseless-cast
                      -Wold-style-cast
                      -Wswitch-enum
                      -Wsign-conversion
                      -Wmissing-declarations
                      -Wunused-but-set-variable
                      -Wunused-result
                      -Wdouble-promotion
                      -Wtrampolines
                      -Wzero-as-null-pointer-constant
                      -Wnull-dereference
                      -Wduplicated-cond
                      -Wduplicated-branches
                      -Wcast-align=strict
                      -Wmissing-include-dirs
                      -Wno-missing-declarations)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_compile_options(/W4
                      /WX)
endif()

add_custom_target(style)
add_custom_command(TARGET style COMMAND find ${CMAKE_CURRENT_LIST_DIR}/benchmark ${CMAKE_CURRENT_LIST_DIR}/example
  ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/test -iname
  "*.hpp" -or -iname "*.cpp" | xargs clang-format -i)

option(ENABLE_MEMCHECK "Run the unit tests and examples under valgrind if it is found." OFF)
option(ENABLE_COVERAGE "Run coverage." OFF)
option(ENABLE_SANITIZERS "Run static analysis." OFF)
option(BUILD_BENCHMARKS "Enable building the benchmarks" ON)
option(BUILD_EXAMPLES "Enable building the examples" ON)
option(BUILD_TESTS "Enable building the tests" ON)

if (ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

if (ENABLE_SANITIZERS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")
endif()

enable_testing()
include_directories(include)

if (BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()
if (BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
if (BUILD_TESTS)
  add_subdirectory(test)
endif()

install(FILES include/boost/ut.hpp DESTINATION include/boost/)
