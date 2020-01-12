//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

namespace ut = boost::ut;

ut::suite _ = [] {
  using namespace ut;

  "test suite"_test = [] {
    "should be equal"_test = [] { expect(42_i == 42); };
  };
};

int main() {
  return ut::cfg<>.run();  // explicitly run registered test suites
}
