#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
option(BOOST_UT_DISABLE_MODULE "Disable ut module" ON)

if(NOT BOOST_UT_DISABLE_MODULE)
  cmake_minimum_required(VERSION 4.0.0)
else()
  cmake_minimum_required(VERSION 3.21...3.25)
endif()
project(
  ut
  VERSION 2.3.1
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
option(BOOST_UT_ENABLE_SANITIZERS "Build with sanitizers" OFF)
option(BOOST_UT_BUILD_BENCHMARKS "Build the benchmarks" OFF)
option(BOOST_UT_BUILD_EXAMPLES "Build the examples" ${PROJECT_IS_TOP_LEVEL})
option(BOOST_UT_BUILD_TESTS "Build the tests" ${PROJECT_IS_TOP_LEVEL})
option(BOOST_UT_ENABLE_INSTALL "Enable install targets" ${PROJECT_IS_TOP_LEVEL})
option(BOOST_UT_USE_WARNINGS_AS_ERRORS "Treat compile warnings as errors" ${PROJECT_IS_TOP_LEVEL})

if(BOOST_UT_USE_WARNINGS_AS_ERRORS)
  include(cmake/WarningsAsErrors.cmake)
endif()

add_custom_target(style COMMENT "Running clang-format")
add_custom_command(
  TARGET style
  POST_BUILD
  COMMAND find ${CMAKE_CURRENT_LIST_DIR}/benchmark ${CMAKE_CURRENT_LIST_DIR}/example ${CMAKE_CURRENT_LIST_DIR}/include
          ${CMAKE_CURRENT_LIST_DIR}/test -iname "*.hpp" -or -iname "*.cpp" | xargs clang-format -i
  COMMENT "Running clang-format"
)

if(BOOST_UT_ENABLE_COVERAGE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3 -O0")
endif()

if(BOOST_UT_ENABLE_SANITIZERS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer
  -fsanitize=address,leak,undefined"
  )
endif()

add_library(ut INTERFACE)
if(NOT TARGET Boost::ut)
  add_library(Boost::ut ALIAS ut)
endif()
target_compile_features(ut INTERFACE cxx_std_20)
target_sources(ut INTERFACE FILE_SET HEADERS BASE_DIRS include FILES include/boost/ut.hpp)
install(
  TARGETS ut
  EXPORT ut_Targets
  FILE_SET HEADERS
)

if(BOOST_UT_DISABLE_MODULE)
  target_compile_definitions(ut INTERFACE BOOST_UT_DISABLE_MODULE)
else()
  include(GNUInstallDirs)

  add_library(ut_module)
  add_library(Boost::ut_module ALIAS ut_module)
  target_link_libraries(ut_module PUBLIC ut)
  target_compile_features(ut_module PUBLIC cxx_std_23)
  set_target_properties(ut_module PROPERTIES CXX_MODULE_STD ON)
  target_sources(ut_module PUBLIC FILE_SET CXX_MODULES BASE_DIRS include FILES include/boost/ut.cppm)
  target_link_libraries(ut_module PUBLIC Boost::ut)
  install(
    TARGETS ut_module
    EXPORT ut_Targets
    FILE_SET CXX_MODULES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
endif()

install(
  EXPORT ut_Targets
  FILE utConfig.cmake
  NAMESPACE Boost::
  DESTINATION lib/cmake/ut
  CXX_MODULES_DIRECTORY "."
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "utConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT
)
install(FILES "${PROJECT_BINARY_DIR}/utConfigVersion.cmake" DESTINATION lib/cmake/ut)

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".js")
  target_link_options(ut INTERFACE "SHELL:-s ALLOW_MEMORY_GROWTH=1" "SHELL:-s EXIT_RUNTIME=1" -fwasm-exceptions -g)
  target_compile_options(ut INTERFACE -fwasm-exceptions -g)
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
