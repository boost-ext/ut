#
# Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(ft ts1.cpp ts2.cpp main.cpp)
include_directories(include)
add_definitions(-include cfg.hpp)
add_custom_command(TARGET ft COMMAND ./ft)
