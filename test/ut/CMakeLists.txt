#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(MEMORYCHECK_COMMAND valgrind)

if (ENABLE_MEMCHECK AND MEMORYCHECK_COMMAND)
  function(ut name)
    add_executable(${name} ${name}.cpp)
    if (ENABLE_RUN_AFTER_BUILD)
      add_custom_command(TARGET ${name} COMMAND ${MEMORYCHECK_COMMAND}
        --leak-check=full --error-exitcode=1 ./${name})
    endif()
  endfunction()
else()
  function(ut name)
    add_executable(${name} ${name}.cpp)
    if (ENABLE_RUN_AFTER_BUILD)
      add_custom_command(TARGET ${name} COMMAND ${name})
    endif()
  endfunction()
endif()

ut(ut)
