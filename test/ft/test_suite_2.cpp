//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include "test.hpp"

static test::suite _ = [] {
  using namespace test;

  "exceptions/expressions"_test = [] {
    "should throw"_test = [] { expect(throws<int>([] { throw 42; })); };
    "compound expression"_test = [] { expect(42_i == 42 and 1 > 0_i); };
  };
};
