#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
include_directories(include)
add_executable(ft test_suite_1.cpp test_suite_2.cpp main.cpp)
add_custom_command(TARGET ft COMMAND ft)
