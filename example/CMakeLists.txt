#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(BOOST_UT_MEMORYCHECK_COMMAND valgrind)

# -- REQUIRED FOR: AppleClang 12.0
# NEEDED-BY: boost::ut
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(STATUS "USING: CXX_COMPILER_EXTRA_OPTIONS in ${CMAKE_CURRENT_SOURCE_DIR}")
    set(example_CXX_COMPILER_EXTRA_OPTIONS
        -Wno-shadow-field
        -Wno-shadow-uncaptured-local
        -Wno-sign-conversion
    )
endif()


function(example file target)
  set(_example_EXTRA_LIBS ${${target}_EXTRA_LIBS})
  if(_example_EXTRA_LIBS)
    message(STATUS "example: ${target}_EXTRA_LIBS= ${_example_EXTRA_LIBS}")
  endif()

  add_executable(boost_ut_${target} ${file}.cpp)
  target_link_libraries(boost_ut_${target} PRIVATE boost::ut ${_example_EXTRA_LIBS})
  if(${target} MATCHES "_cpp17$")
    set_property(TARGET boost_ut_${target} PROPERTY CXX_STANDARD 17)
  endif()
  if(BOOST_UT_ENABLE_MEMCHECK AND BOOST_UT_MEMORYCHECK_COMMAND)
    ut_add_custom_command_or_test(TARGET boost_ut_${target} COMMAND ${BOOST_UT_MEMORYCHECK_COMMAND}
                                  --leak-check=full --error-exitcode=1 ./boost_ut_${target} ${ARGN})
    add_test(NAME example.memcheck.${target}
        COMMAND ${BOOST_UT_MEMORYCHECK_COMMAND} --leak-check=full --error-exitcode=1 ./boost_ut_${target} ${ARGN}
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
  else()
    ut_add_custom_command_or_test(TARGET boost_ut_${target} COMMAND boost_ut_${target} ${ARGN})
  endif()
  add_test(NAME example.${target} COMMAND boost_ut_${target} ${ARGN})
endfunction()


example(cfg/printer printer)
example(cfg/runner runner)

if (DEFINED ENV{TBB_ROOT})
  set(TBB_ROOT $ENV{TBB_ROOT})
  include(${TBB_ROOT}/cmake/TBBBuild.cmake)
  tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR MAKE_ARGS stdver="c++17")
  find_package(TBB)
  # DISABLED: target_link_libraries(boost_ut_parallel_runner PRIVATE ${TBB_IMPORTED_TARGETS})
  set(parallel_runner_EXTRA_LIBS ${TBB_IMPORTED_TARGETS})
endif()
example(cfg/parallel_runner parallel_runner)

example(cfg/reporter reporter)

if (NOT WIN32)
  example(abort abort)
endif()

example(BDD BDD)
example(benchmark benchmark)
example(cli cli_pass "cli.pass")
example(cli cli_pass_no_colors "cli.pass" "0" "1")
example(cli cli_pass_dry_run "cli.pass" "1" "1")
example(cli cli_pass_not_dry_run "cli.pass" "1" "0")
example(cli cli_all_dry_run "\\*" "1" "1")
example(cli cli_fail_dry_run "cli.fail" "1" "1")
example(exception exception)
example(expect expect)
example(fatal fatal)
example(filter filter)

if(NOT WIN32 AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  example(gherkin gherkin)
  example(gherkin gherkin_feature "../../example/gherkin.feature")

  # -- SPECIALIZE COMPILER-OPTIONS:
  # NEEDED-BY: boost::ut
  target_compile_options(boost_ut_gherkin         PRIVATE ${example_CXX_COMPILER_EXTRA_OPTIONS})
  target_compile_options(boost_ut_gherkin_feature PRIVATE ${example_CXX_COMPILER_EXTRA_OPTIONS})
endif()
example(hello_world hello_world)
example(log log)
example(macro macro)
example(main main '')
example(minimal minimal)
example(mut mut)
example(matcher matcher)

if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
  example(parameterized parameterized)
  target_compile_options(boost_ut_parameterized  PRIVATE ${example_CXX_COMPILER_EXTRA_OPTIONS})
endif()

example(run run)
example(run_report run_report)
example(section section)
example(should should)
example(skip skip)
example(spec spec)
example(suite suite)
example(tag tag)
example(terse terse)
example(test _test)
example(tmp tmp)
example(using using)

if(BUILD_UT_ENABLE_CPP17_EXAMPLES)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR
    "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  example(expect expect_cpp17)
  example(test test_cpp17)
  example(suite suite_cpp17)
  example(tag tag_cpp17)
  example(terse terse_cpp17)
  example(section section_cpp17)
  example(should should_cpp17)
  example(skip skip_cpp17)
endif()
endif()

# -- SPECIALIZE COMPILER-OPTIONS:
# NEEDED-BY: boost::ut
target_compile_options(boost_ut_benchmark  PRIVATE ${example_CXX_COMPILER_EXTRA_OPTIONS})
