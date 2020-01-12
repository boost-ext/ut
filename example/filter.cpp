//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  cfg<override> = {.filter = "run.sub1"};

  "run"_test = [] {
    "sub1"_test = [] { expect(42 == 42_i); };
    "sub2"_test = [] { expect(43 == 42_i); };
  };

  "don't run"_test = [] { expect(0 == 1_i) << "don't run"; };
}
