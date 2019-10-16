#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(MEMORYCHECK_COMMAND valgrind)

if (ENABLE_MEMCHECK AND MEMORYCHECK_COMMAND)
  function(example name)
    string(REPLACE "/" "_" exe ${name})
    add_executable(${exe} ${name}.cpp)
    add_custom_command(TARGET ${exe} COMMAND ${MEMORYCHECK_COMMAND}
      --leak-check=full --error-exitcode=1 ./${exe})
  endfunction()
else()
  function(example name)
    string(REPLACE "/" "_" exe ${name})
    add_executable(${exe} ${name}.cpp)
    add_custom_command(TARGET ${exe} COMMAND ${exe})
  endfunction()
endif()

example(cfgs/example)
example(cfgs/run)
example(assertions)
example(BDD)
example(exceptions)
example(hello_world)
example(logging)
example(parameterized)
example(sections)
example(skip)
example(suites)
