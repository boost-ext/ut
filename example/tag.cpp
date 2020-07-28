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

  cfg<override> = {.filter = "tag", .tag = {"execute"}};

  // clang-format off
  tag("execute") / skip /
  "tag"_test = [] {
    expect(42_i == 43) << "should not fire!";
    expect(false) << "should fail!";
  };

  tag("execute") / "tag"_test= [] {
    expect(42_i == 42);
  };

  tag("not executed") / "tag"_test= [] {
    expect(43_i == 42);
  };

  tag("not executed") / tag("execute") /
  "tag"_test= [] {
    expect(42_i == 42);
  };
  // clang-format on
}
