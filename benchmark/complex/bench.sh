#!/bin/bash

#SUT=doctest
#SUT=catch2
SUT=boost/ut

function clean {
  rm -f test_suite_*.cpp *.o *.out *.pch *.pcm
}

function generate {
  for ((v=0; v<100; ++v)); do
    (echo '<% n=100; v=' $v '%>' && cat $SUT/test_suite.erb) | erb > test_suite_$v.cpp
  done
}

function compile {
  for ((v=0; v<100; ++v)); do
    clang++ -c -I. -std=c++2a -stdlib=libc++ test_suite_$v.cpp --gcc-toolchain=/lib/gcc9/9.2.0
  done
  clang++ -I. -std=c++2a -stdlib=libc++ $SUT/main.cpp *.o --gcc-toolchain=/lib/gcc9/9.2.0
}

function compile_pch {
  clang++ -I. -std=c++2a -stdlib=libc++ --gcc-toolchain=/lib/gcc9/9.2.0 $SUT.hpp -o sut.pch
  for ((v=0; v<100; ++v)); do
    clang++ -c -include-pch sut.pch -H -I. -std=c++2a -stdlib=libc++ test_suite_$v.cpp --gcc-toolchain=/lib/gcc9/9.2.0
  done
  clang++ -I. -std=c++2a -stdlib=libc++ $SUT/main.cpp *.o --gcc-toolchain=/lib/gcc9/9.2.0
}

function compile_modules {
  clang++ -I. -std=c++2a -stdlib=libc++ --gcc-toolchain=/lib/gcc9/9.2.0 -D__cpp_modules -fmodules --precompile -x c++-module $SUT.hpp -o boost.ut.pcm
  for ((v=0; v<100; ++v)); do
    clang++ -c -I. -std=c++2a -stdlib=libc++ test_suite_$v.cpp --gcc-toolchain=/lib/gcc9/9.2.0 -D__cpp_modules -fmodules -fprebuilt-module-path=.
  done
  clang++ -I. -std=c++2a -stdlib=libc++ $SUT/main.cpp *.o --gcc-toolchain=/lib/gcc9/9.2.0
}

clean
generate
time `compile`
#time `compile_pch`
#time `compile_modules`
time ./a.out
