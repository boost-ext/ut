//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using boost::ut::operator""_test;
  using boost::ut::eq;
  using boost::ut::expect;
  using boost::ut::neq;

  "using"_test = [] {
    expect(eq(42, 42));
    expect(neq(1, 2));
  };
}
