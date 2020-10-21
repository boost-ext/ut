#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

find_program(BOOST_UT_MEMORYCHECK_COMMAND valgrind)

# -- REQUIRED FOR: AppleClang 12.0, clang-10, gcc-10, ...
# NEEDED-BY: boost::ut
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(STATUS "USING: CXX_COMPILER_EXTRA_OPTIONS in ${CMAKE_CURRENT_SOURCE_DIR}")
    set(boost_ut_executable_CXX_COMPILER_EXTRA_OPTIONS
        -Wno-missing-noreturn
        -Wno-shadow-field-in-constructor
        -Wno-shadow-uncaptured-local
        -Wno-double-promotion
        -Wno-float-equal
        -Wno-error=float-equal
        -Wno-sign-conversion
        -Wno-error=sign-conversion
        -Wno-implicit-float-conversion
        # DISABLED: -Wno-implicit-int-float-conversion
    )
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    message(STATUS "USING: CXX_COMPILER_EXTRA_OPTIONS in ${CMAKE_CURRENT_SOURCE_DIR}")
    set(boost_ut_executable_CXX_COMPILER_EXTRA_OPTIONS
        -Wno-sign-conversion
        -Wno-error=sign-conversion
        -Wno-float-equal
        -Wno-error=float-equal
    )
endif()


if (BOOST_UT_ENABLE_MEMCHECK AND BOOST_UT_MEMORYCHECK_COMMAND)
  message(STATUS "USING: MEMCHECK enabled (using: ${BOOST_UT_MEMORYCHECK_COMMAND})")
  function(boost_ut_executable name)
    add_executable(test_${name} ${name}.cpp)
    target_link_libraries(test_${name} PRIVATE boost::ut)
    target_compile_options(test_${name} PRIVATE ${boost_ut_executable_CXX_COMPILER_EXTRA_OPTIONS})
    ut_add_custom_command_or_test(TARGET test_${name} COMMAND ${BOOST_UT_MEMORYCHECK_COMMAND}
        --leak-check=full --error-exitcode=1 ./test_${name})
    add_test(NAME test.${name} COMMAND test_${name})
    add_test(NAME test.memcheck.${name}
        COMMAND ${BOOST_UT_MEMORYCHECK_COMMAND} --leak-check=full --error-exitcode=1 ./test_${name}
    )
  endfunction()
else()
  function(boost_ut_executable name)
    add_executable(test_${name} ${name}.cpp)
    target_link_libraries(test_${name} PUBLIC boost::ut)
    target_compile_options(test_${name} PRIVATE ${boost_ut_executable_CXX_COMPILER_EXTRA_OPTIONS})
    ut_add_custom_command_or_test(TARGET test_${name} COMMAND test_${name})
    add_test(NAME test.${name} COMMAND test_${name})
  endfunction()
endif()

boost_ut_executable(ut)
