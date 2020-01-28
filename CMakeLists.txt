#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.0)
project(boost.ut CXX)
set(CMAKE_CXX_STANDARD 20)

add_library(boost.ut INTERFACE)
target_include_directories(boost.ut INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if (WIN32) # clang-cl
    target_compile_options(boost.ut INTERFACE
                        -Weverything
                        -Werror
                        -pedantic
                        -pedantic-errors
                        -Wno-c++98-compat
                        -Wno-c++98-compat-pedantic
                        -Wno-c99-extensions)
  else()
    target_compile_options(boost.ut INTERFACE
                        -Weverything
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
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  target_compile_options(boost.ut INTERFACE
                      -Wall
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
  target_compile_options(boost.ut INTERFACE
                      /W4
                      /WX)
endif()

add_custom_target(style)
add_custom_command(TARGET style COMMAND find ${CMAKE_CURRENT_LIST_DIR}/benchmark ${CMAKE_CURRENT_LIST_DIR}/example
  ${CMAKE_CURRENT_LIST_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/test -iname
  "*.hpp" -or -iname "*.cpp" | xargs clang-format -i)

option(ENABLE_MEMCHECK "Run the unit tests and examples under valgrind if it is found" OFF)
option(ENABLE_COVERAGE "Run coverage" OFF)
option(ENABLE_SANITIZERS "Run static analysis" OFF)
option(ENABLE_RUN_AFTER_BUILD "Automatically run built artifacts. If disabled, the tests can be run with ctest instead" ON)
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

function(ut_add_custom_command_or_test)
  # Define the supported set of keywords
  set(prefix "PARSE")
  set(noValues "")
  set(singleValues TARGET)
  set(multiValues COMMAND)

  # Process the arguments passed in
  include(CMakeParseArguments)
  cmake_parse_arguments(
          "${prefix}"
          "${noValues}"
          "${singleValues}"
          "${multiValues}"
          ${ARGN})

  if (ENABLE_RUN_AFTER_BUILD)
    add_custom_command(TARGET ${PARSE_TARGET} COMMAND ${PARSE_COMMAND})
  else()
    add_test(NAME ${PARSE_TARGET} COMMAND ${PARSE_COMMAND})
  endif()
endfunction()

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
