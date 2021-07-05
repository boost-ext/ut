#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
find_program(BOOST_UT_MEMORYCHECK_COMMAND valgrind)

function(example file target)
  add_executable(boost_ut_${target} ${file}.cpp)
  if(${target} MATCHES "_cpp17$")
    set_property(TARGET boost_ut_${target} PROPERTY CXX_STANDARD 17)
  endif()
  if(BOOST_UT_ENABLE_MEMCHECK AND BOOST_UT_MEMORYCHECK_COMMAND)
    ut_add_custom_command_or_test(
      TARGET
      boost_ut_${target}
      COMMAND
      ${BOOST_UT_MEMORYCHECK_COMMAND}
      --leak-check=full
      --error-exitcode=1
      ./boost_ut_${target}
      ${ARGN}
    )
  else()
    ut_add_custom_command_or_test(TARGET boost_ut_${target} COMMAND boost_ut_${target} ${ARGN})
  endif()
endfunction()

example(cfg/printer printer)
example(cfg/runner runner)

if(APPLE OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  example(cfg/parallel_runner parallel_runner)
endif()

# if(DEFINED ENV{TBB_ROOT})
#   set(TBB_ROOT $ENV{TBB_ROOT})
#   # FIXME: include could not find load file:
#   include(${TBB_ROOT}/cmake/TBBBuild.cmake)
#   tbb_build(TBB_ROOT ${TBB_ROOT} CONFIG_DIR TBB_DIR MAKE_ARGS stdver="c++17")
#   find_package(TBB)
#   target_link_libraries(boost_ut_parallel_runner ${TBB_IMPORTED_TARGETS})
# endif()

example(cfg/reporter reporter)

if(NOT WIN32)
  example(abort abort)
endif()

example(attr attr)
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

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  example(expect expect_cpp17)
  example(test test_cpp17)
  example(suite suite_cpp17)
  example(tag tag_cpp17)
  example(terse terse_cpp17)
  example(section section_cpp17)
  example(should should_cpp17)
  example(skip skip_cpp17)
endif()
