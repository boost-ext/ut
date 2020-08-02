#
# Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
CXXFLAGS=-stdlib=libc++ -std=c++2a -D__cpp_modules -fmodules -Wall -Wextra -Werror -pedantic -pedantic-errors -Wno-unused-comparison

all: clean
	clang++ ${CXXFLAGS} --precompile -x c++-module ut.hpp -o ut.pcm
	clang++ ${CXXFLAGS} -fprebuilt-module-path=. main.cpp ut.pcm
	./a.out

clean:
	rm -f a.out
