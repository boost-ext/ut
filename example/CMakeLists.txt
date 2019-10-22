#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(MEMORYCHECK_COMMAND valgrind)

if (ENABLE_MEMCHECK AND MEMORYCHECK_COMMAND)
  function(example name)
    add_executable(${name} ${name}.cpp)
    add_custom_command(TARGET ${name} COMMAND ${MEMORYCHECK_COMMAND}
      --leak-check=full --error-exitcode=1 ./${name})
  endfunction()
else()
  function(example name)
    add_executable(${name} ${name}.cpp)
    add_custom_command(TARGET ${name} COMMAND ${name})
  endfunction()
endif()

example(assertions)
example(BDD)
example(cfg)
example(exceptions)
example(filter)
example(hello_world)
example(logging)
example(parameterized)
example(run)
example(sections)
example(skip)
example(suites)
