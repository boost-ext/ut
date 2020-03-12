//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

constexpr auto sum = [](auto... args) { return (0 + ... + args); };

int main() {
  using boost::ut::operator""_test;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;

  "terse"_test = [] {
    42_i == sum(40, 2);
    2_i == sum(1, 1);
    sum(1, 1, 2) == 4_i;
    42_i == sum(40, 2) and 2_i == sum(2) and 3_i == 3;
    3_i == sum(1, 1, 2);
    sum(1, 2, 3) == 6_i;
  };
}
