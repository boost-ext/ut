#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#

function(benchmark name file)
  add_executable(benchmark_${name} ${file}.cpp)
  target_link_libraries(benchmark_${name} PRIVATE boost::ut)
  # target_compile_options(benchmark_${name} PRIVATE ${ARGN})
  target_compile_definitions(benchmark_${name} PRIVATE ${ARGN})
  ut_add_custom_command_or_test(TARGET benchmark_${name} COMMAND benchmark_${name})
  add_test(NAME benchmark.${name} COMMAND benchmark_${name})
endfunction()

benchmark(expect_udl expect "-DEXPECT_UDL")
benchmark(expect_that expect "-DEXPECT_THAT")
benchmark(expect_eq expect "-DEXPECT_EQ")
benchmark(include include)
benchmark(suite suite)
benchmark(test test)
