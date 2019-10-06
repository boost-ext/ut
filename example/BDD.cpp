//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using namespace ut;

  "scenario"_test = [] {
    given("I have...") = [] {
      when("I run...") = [] {
        then("I expect...") = [] { expect(1_u == 1u); };
        then("I expect...") = [] { expect(1u == 1_u); };
      };
    };
  };
}
