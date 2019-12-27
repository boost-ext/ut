//
// Copyright (c) 2019 Kris Jusiak (kris at jusiak dot net)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/ut.hpp>
#include <tuple>
#include <type_traits>
#include <vector>

int main() {
  using namespace boost::ut;

  for (const auto& i : std::vector{1, 2, 3}) {
    test("parameterized " + std::to_string(i)) = [i] {  // 3 tests
      expect(that % i > 0);                             // 3 asserts
    };
  }

  "args"_test = [](const auto& arg) {
    expect(arg > 0_i) << "all values greater than 0";
  } | std::vector{1, 2, 3};

  "types"_test = []<class T>() {
    expect(std::is_integral_v<T>) << "all types are integrals";
  }
  | std::tuple<bool, int>{};

  "args and types"_test = []<class TArg>(const TArg& arg) {
    !expect(std::is_integral_v<TArg>);
    expect(42_i == static_cast<int>(arg) or arg);
    expect(type<TArg> == type<int> or type<TArg> == type<bool>);
  }
  | std::tuple{42, true};
}
