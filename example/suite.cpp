//
// Copyright (c) 2019-2020 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>

namespace ut = boost::ut;

ut::suite errors = [] {
  using namespace ut;

  "exception"_test = [] {
    expect(throws([] { throw 0; })) << "throws any exception";
  };

  "failure"_test = [] { expect(nothrow([] {})); };
};

#if not defined(__clang__) and not defined(_MSC_VER)
struct calc {
  static inline ut::suite sum_suite = []<class T = calc>()->void {
    using namespace ut;

    "sum"_test = [] {
      T calc{};
      expect(6_i == calc.sum(1, 2, 3));
    };
  };

  template <class... TArgs>
  constexpr auto sum(TArgs... args) -> int {
    return (args + ...);
  }
};
#endif

int main() {}
