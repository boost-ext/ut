#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(BOOST_UT_MEMORYCHECK_COMMAND valgrind)

if(BOOST_UT_ENABLE_MEMCHECK AND BOOST_UT_MEMORYCHECK_COMMAND)
  function(ut name)
    add_executable(${name}_test ${name}.cpp)
    ut_add_custom_command_or_test(
      TARGET
      ${name}_test
      COMMAND
      ${BOOST_UT_MEMORYCHECK_COMMAND}
      --leak-check=full
      --error-exitcode=1
      ./${name}_test
    )
  endfunction()
else()
  function(ut name)
    add_executable(${name}_test ${name}.cpp)
    ut_add_custom_command_or_test(TARGET ${name}_test COMMAND ${name}_test)
  endfunction()
endif()

ut(ut)
