//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

auto* ptr() {
  static auto i = 42;  /// Change i to 0 for fatal assertions
  return &i;
}

int main() {
  using boost::ut::operator""_test;
  using boost::ut::expect;
  using namespace boost::ut::literals;

  "fatal"_test = [] {
    using namespace boost::ut;
    !expect(ptr());
    expect(*ptr() != 0_i);
  };

  "fatal terse"_test = [] {
    using namespace boost::ut::operators::terse;
    !expect(ptr()) and *ptr() != 0_i;
  };
}
