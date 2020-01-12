#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(MEMORYCHECK_COMMAND valgrind)

if (ENABLE_MEMCHECK AND MEMORYCHECK_COMMAND)
  function(ut name)
    add_executable(${name} ${name}.cpp)
    add_custom_command(TARGET ${name} COMMAND ${MEMORYCHECK_COMMAND}
      --leak-check=full --error-exitcode=1 ./${name})
  endfunction()
else()
  function(ut name)
    add_executable(${name} ${name}.cpp)
    add_custom_command(TARGET ${name} COMMAND ${name})
  endfunction()
endif()

if (NOT WIN32 OR (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC"))
  ut(ut)
endif()
