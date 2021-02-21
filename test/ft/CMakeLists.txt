#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
include_directories(include)

add_executable(ft main.cpp test_suite_1.cpp test_suite_2.cpp test_suite_3.cpp)
ut_add_custom_command_or_test(TARGET ft COMMAND ft)

add_executable(ft-no-exceptions main.cpp test_suite_1.cpp test_suite_2.cpp test_suite_3.cpp)
ut_add_custom_command_or_test(TARGET ft-no-exceptions COMMAND ft-no-exceptions)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if(WIN32)
    set_source_files_properties(test_suite_1.cpp PROPERTIES COMPILE_FLAGS /EHsc)
    set_source_files_properties(test_suite_3.cpp PROPERTIES COMPILE_FLAGS /EHsc)
  else()
    set_source_files_properties(test_suite_1.cpp PROPERTIES COMPILE_FLAGS -fno-exceptions)
    set_source_files_properties(test_suite_3.cpp PROPERTIES COMPILE_FLAGS -fno-exceptions)
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set_source_files_properties(test_suite_1.cpp PROPERTIES COMPILE_FLAGS -fno-exceptions)
  set_source_files_properties(test_suite_3.cpp PROPERTIES COMPILE_FLAGS -fno-exceptions)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set_source_files_properties(test_suite_1.cpp PROPERTIES COMPILE_FLAGS /EHsc)
  set_source_files_properties(test_suite_3.cpp PROPERTIES COMPILE_FLAGS /EHsc)
endif()

if(NOT WIN32) # WIN32 includes both MSVC and clang-cl
  add_executable(ft-link main.cpp test_suite_1.cpp test_suite_2.cpp test_suite_3.cpp)
  ut_add_custom_command_or_test(TARGET ft-link COMMAND ft-link)
endif()
