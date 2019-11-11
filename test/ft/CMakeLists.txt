#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
include_directories(include)

add_executable(ft test_suite_1.cpp test_suite_2.cpp test_suite_3.cpp main.cpp)
add_custom_command(TARGET ft COMMAND ft)

add_executable(ft-no-exceptions test_suite_1.cpp test_suite_2.cpp test_suite_3.cpp main.cpp)
add_custom_command(TARGET ft-no-exceptions COMMAND ft-no-exceptions)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_source_files_properties(test_suite_1.cpp PROPERTIES COMPILE_FLAGS -fno-exceptions)
  set_source_files_properties(test_suite_3.cpp PROPERTIES COMPILE_FLAGS -fno-exceptions)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set_source_files_properties(test_suite_1.cpp PROPERTIES COMPILE_FLAGS /EHsc)
  set_source_files_properties(test_suite_3.cpp PROPERTIES COMPILE_FLAGS /EHsc)
endif()
