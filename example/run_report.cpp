//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <iostream>

namespace ut = boost::ut;

ut::suite basic = [] {
  using namespace ut;
  should("equal") = [] { expect(42_i == 42); };
};

int main() {
  const auto result = ut::cfg<>.run(
      {.report_errors =
           true});  // explicitly run registered test suites and report errors
  std::cout << "After report\n";
  return result;
}
