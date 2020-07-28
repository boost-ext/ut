//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

int main() {
  using namespace boost::ut;

  auto i = 0;  // mutable

  "mut"_test = [i] {
    expect((i == 0_i) >> fatal);  // immutable

    should("++") = [i] {
      expect(++mut(i) == 1_i);  // mutable
      expect(i == 1_i);         // immutable
    };

    should("--") = [i] {
      expect(--mut(i) == -1_i);  // mutable
      expect(i == -1_i);         // immutable
    };
  };
}
