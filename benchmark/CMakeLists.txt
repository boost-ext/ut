#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
function(benchmark name)
  add_executable(b${name} ${name}.cpp)
  add_custom_command(TARGET b${name} COMMAND b${name})
endfunction()

benchmark(assert)
benchmark(include)
benchmark(test)
