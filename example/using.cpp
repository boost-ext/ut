//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  namespace ut = boost::ut;
  using ut::operator""_test;

  "using"_test = [] {
    ut::expect(ut::eq(42, 42));
    ut::expect(ut::neq(1, 2));
  };
}
