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
  using namespace boost::ut::operators;
  using namespace boost::ut::literals;
  using namespace boost::ut::spec;
  using boost::ut::expect;

  describe("sum") = [] {
    it("should be 0") = [] { expect(sum() == 0_i); };
    it("should add all args") = [] { expect(sum(1, 2, 3) == 6_i); };
  };
}
