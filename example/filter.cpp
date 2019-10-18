//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  /**
   * Alternatively set env variable `BOOST_UT_FILTER` to "filter"
   */
  cfg<override>.filter = "filter";

  "filter"_test = [] {
    expect(42 == 42_i);
  };

  "don't run"_test = [] {
    expect(0 == 1_c) << "don't run";
  };
}
