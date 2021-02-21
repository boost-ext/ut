#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.14...3.20)
project(boost.ut.test LANGUAGES CXX)

if(TEST_INSTALLED_VERSION)
  find_package(boost_ut 1.1 REQUIRED)
  include(../cmake/AddCustomCommandOrTest.cmake)
  enable_testing()
endif()

add_subdirectory(ut)
add_subdirectory(ft)
