#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.12...3.23)
project(boost.ut.test LANGUAGES CXX)

if(TEST_INSTALLED_VERSION)
  if(NOT DEFINED CMAKE_CXX_STANDARD)
    option(CXX_STANDARD_REQUIRED "Require c++ standard" YES)
    if(NOT DEFINED CMAKE_CXX_STANDARD)
      set(CMAKE_CXX_STANDARD
          20
          CACHE STRING "Default value for CXX_STANDARD property of targets."
      )
      option(CMAKE_CXX_STANDARD_REQUIRED "Default value for CXX_STANDARD_REQUIRED property of targets." YES)
      option(CMAKE_CXX_EXTENSIONS "Default value for CXX_EXTENSIONS property of targets." NO)
    endif()
  endif()

  find_package(ut REQUIRED)
  include(../cmake/AddCustomCommandOrTest.cmake)

  enable_testing()
endif()

add_subdirectory(ut)
add_subdirectory(ft)
