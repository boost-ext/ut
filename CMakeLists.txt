#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.1)
project(boost.ut CXX)

include(CTest)

set(MASTER_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MASTER_PROJECT ON)
endif()

set(CMAKE_CXX_STANDARD 20 CACHE STRING "Default value for CXX_STANDARD property of targets.")
set(CMAKE_CXX_STANDARD_REQUIRED yes CACHE BOOL "Default value for CXX_STANDARD_REQUIRED property of targets.")
set(CMAKE_CXX_EXTENSIONS no CACHE BOOL "Default value for CXX_EXTENSIONS property of targets.")
option(BOOST_UT_ENABLE_MEMCHECK "Run the unit tests and examples under valgrind if it is found" OFF)
option(BOOST_UT_ENABLE_COVERAGE "Run coverage" OFF)
option(BOOST_UT_ENABLE_SANITIZERS "Run static analysis" OFF)
option(BOOST_UT_ENABLE_RUN_AFTER_BUILD "Automatically run built artifacts. If disabled, the tests can be run with ctest instead" ON)
option(BOOST_UT_BUILD_BENCHMARKS "Build the benchmarks" ${MASTER_PROJECT})
option(BOOST_UT_BUILD_EXAMPLES "Build the examples" ${MASTER_PROJECT})
option(BOOST_UT_BUILD_TESTS "Build the tests" ${MASTER_PROJECT})

add_library(boost.ut INTERFACE)
target_include_directories(boost.ut INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
add_library(boost::ut ALIAS boost.ut)

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

if (BOOST_UT_ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

if (BOOST_UT_ENABLE_SANITIZERS)
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

  if (BOOST_UT_ENABLE_RUN_AFTER_BUILD)
    add_custom_command(TARGET ${PARSE_TARGET} COMMAND ${PARSE_COMMAND})
  else()
    add_test(NAME ${PARSE_TARGET} COMMAND ${PARSE_COMMAND})
  endif()
endfunction()

include_directories(include)

if (BOOST_UT_BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()
if (BOOST_UT_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
if (BOOST_UT_BUILD_TESTS)
  add_subdirectory(test)
endif()

## Packaging.
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(package_directory "${CMAKE_INSTALL_PREFIX}/lib/cmake/ut")
set(config_output "${CMAKE_CURRENT_BINARY_DIR}/ut-config.cmake")

# Generate config file.
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/ut-config.cmake.in"
  "${config_output}"
  INSTALL_DESTINATION "${package_directory}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Install includes, config and targets.
install(
  FILES include/boost/ut.hpp
  DESTINATION include/boost/
)
install(
  FILES "${config_output}"
  DESTINATION "${package_directory}"
)
