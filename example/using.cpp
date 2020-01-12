//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using boost::ut::operator""_test;
  using boost::ut::operator""_i;

  "using"_test = [] {
    using boost::ut::expect;

    using boost::ut::eq;
    expect(eq(42, 42));

    using boost::ut::operator==;
    expect(42_i == 42);

    using boost::ut::operator and;
    using boost::ut::that;
    expect(that % 1 == 1 and that % 2 == 2);
  };
}
