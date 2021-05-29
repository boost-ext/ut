#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.12...3.20)
project(
  ut
  VERSION 1.1.8
  LANGUAGES CXX
)

set(MASTER_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MASTER_PROJECT ON)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD
      20
      CACHE STRING "Default value for CXX_STANDARD property of targets."
  )
  option(CMAKE_CXX_STANDARD_REQUIRED "Default value for CXX_STANDARD_REQUIRED property of targets." YES)
  option(CMAKE_CXX_EXTENSIONS "Default value for CXX_EXTENSIONS property of targets." NO)
endif()

option(BOOST_UT_ENABLE_MEMCHECK "Run the unit tests and examples under valgrind if it is found" OFF)
option(BOOST_UT_ENABLE_COVERAGE "Run coverage" OFF)
option(BOOST_UT_ENABLE_SANITIZERS "Run static analysis" OFF)
option(BOOST_UT_BUILD_BENCHMARKS "Build the benchmarks" ${MASTER_PROJECT})
option(BOOST_UT_BUILD_EXAMPLES "Build the examples" ${MASTER_PROJECT})
option(BOOST_UT_BUILD_TESTS "Build the tests" ${MASTER_PROJECT})
option(BOOST_UT_DISABLE_MODULE "Disable ut module" OFF)

# ---- Add dependencies via CPM ----
# see https://github.com/cpm-cmake/CPM.cmake for more info

if(CMAKE_VERSION VERSION_LESS 3.20.0)
  # see https://github.com/TheLartians/PackageProject.cmake/pull/19
  include(cmake/PackageProject.cmake)
else()
  include(cmake/CPM.cmake)

  # PackageProject.cmake will be used to make our target installable
  CPMAddPackage(
    NAME PackageProject.cmake
    GITHUB_REPOSITORY TheLartians/PackageProject.cmake
    VERSION 1.6.0
  )
endif()

add_library(ut INTERFACE)

set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME}-${PROJECT_VERSION}/include)
# XXX variant: set(INCLUDE_INSTALL_DIR include)
target_include_directories(ut INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>)
if(APPLE)
  target_compile_features(ut INTERFACE cxx_std_20)
else()
  target_compile_features(ut INTERFACE cxx_std_17)
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if(WIN32) # clang-cl
    # FIXME: we should not export this pedantic options! CK
    target_compile_options(
      ut
      INTERFACE -Wall
                -Wextra
                # FIXME -Werror
                -Wno-c++98-c++11-c++14-c++17-compat-pedantic
                -Wno-c++98-c++11-compat
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-c99-extensions
                -Wno-pedantic
    )
  else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  # FIXME: we should not export this pedantic options! CK
  target_compile_options(
    ut
    INTERFACE -Wall
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
              # FIXME
              -Wno-undef
              -Wno-missing-declarations
              -Wno-sign-conversion
              -Wno-float-equal
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_compile_options(/W4 /WX)
endif()

add_custom_target(style)
add_custom_command(
  TARGET style COMMAND find ${CMAKE_CURRENT_LIST_DIR}/benchmark ${CMAKE_CURRENT_LIST_DIR}/example ${CMAKE_CURRENT_LIST_DIR}/include
                       ${CMAKE_CURRENT_LIST_DIR}/test -iname "*.hpp" -or -iname "*.cpp" | xargs clang-format -i
)

if(BOOST_UT_ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif()

if(BOOST_UT_ENABLE_SANITIZERS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak -fsanitize=undefined")
endif()

if(BOOST_UT_DISABLE_MODULE)
  target_compile_definitions(ut INTERFACE BOOST_UT_DISABLE_MODULE)
endif()

# Create target Boost::ut and install target
packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE Boost
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION ${INCLUDE_INSTALL_DIR}
  # XXX variant: DISABLE_VERSION_SUFFIX YES
  # Optional VERSION_HEADER ${VERSION_HEADER_LOCATION}
  COMPATIBILITY SameMajorVersion
  # Note: only if needed i.e. DEPENDENCIES "fmt 7.1.3; span"
)

# Note: now we can use the target Boost::ut
include(cmake/AddCustomCommandOrTest.cmake)

if(BOOST_UT_BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()
if(BOOST_UT_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()
if(BOOST_UT_BUILD_TESTS)
  enable_testing()

  add_subdirectory(test)
endif()
