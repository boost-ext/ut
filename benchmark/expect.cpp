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

  static constexpr auto iterations = 1'000'000;

#if defined(EXPECT_UDL)
  "expect_udl"_test = [] {
    for (auto i = 0; i < iterations; ++i) {
      expect(i == _i(i));
    }
  };
#elif defined(EXPECT_THAT)
  "expect_that"_test = [] {
    for (auto i = 0; i < iterations; ++i) {
      expect(that % i == i);
    }
  };
#elif defined(EXPECT_EQ)
  "expect_eq"_test = [] {
    for (auto i = 0; i < iterations; ++i) {
      expect(eq(i, i));
    }
  };
#endif
}
