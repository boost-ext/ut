#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.21...3.25)
project(
  ut
  VERSION 1.1.9
  LANGUAGES CXX
)

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
option(BOOST_UT_BUILD_BENCHMARKS "Build the benchmarks" OFF)
option(BOOST_UT_BUILD_EXAMPLES "Build the examples" ${PROJECT_IS_TOP_LEVEL})
option(BOOST_UT_BUILD_TESTS "Build the tests" ${PROJECT_IS_TOP_LEVEL})
option(BOOST_UT_USE_WARNINGS_AS_ERORS "Build the tests" ${PROJECT_IS_TOP_LEVEL})
option(BOOST_UT_DISABLE_MODULE "Disable ut module" OFF)
option(BOOST_UT_ALLOW_CPM_USE "Do not reach out across network for CPM" ON)

if(NOT CMAKE_SKIP_INSTALL_RULES)
  if(BOOST_UT_ALLOW_CPM_USE)
    # ---- Add dependencies via CPM ----
    # see https://github.com/cpm-cmake/CPM.cmake for more info

    include(cmake/CPM.cmake)

    # PackageProject.cmake will be used to make our target installable
    CPMAddPackage(
      NAME PackageProject.cmake
      GITHUB_REPOSITORY TheLartians/PackageProject.cmake
      VERSION 1.10.0
    )
  else()
    include(cmake/PackageProject.cmake)
  endif()
endif()

add_library(ut INTERFACE)

if(BOOST_UT_ALLOW_CPM_USE)
add_library(Boost::ut ALIAS ut)
endif()

if(NOT DEFINED INCLUDE_INSTALL_DIR)
  set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME}-${PROJECT_VERSION}/include)
endif()
target_include_directories(ut INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>)
target_compile_features(ut INTERFACE cxx_std_20)

if(BOOST_UT_USE_WARNINGS_AS_ERORS)
  include(cmake/WarningsAsErrors.cmake)
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

if(NOT CMAKE_SKIP_INSTALL_RULES)
  # Create target Boost::ut and install target
  packageProject(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    NAMESPACE Boost
    BINARY_DIR ${PROJECT_BINARY_DIR}
    INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
    INCLUDE_DESTINATION ${INCLUDE_INSTALL_DIR}
    # XXX variant: DISABLE_VERSION_SUFFIX YES
    COMPATIBILITY SameMajorVersion
    # Note: only if needed i.e. DEPENDENCIES "fmt 7.1.3; span"
  )
endif()

if (EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".js")
  target_link_options(ut INTERFACE
          "SHELL:-s ALLOW_MEMORY_GROWTH=1"
          "SHELL:-s EXIT_RUNTIME=1"
          -fwasm-exceptions
          -g
          )
  target_compile_options(ut INTERFACE
          -fwasm-exceptions
          -g
          )
endif()

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
